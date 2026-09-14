/// @file BusScanner.cpp
/// @brief Implementation of asynchronous Pelco-D bus address discovery.

#include "BusScanner.h"

#include "PelcoDFrame.h"
#include "ProtocolBuilder.h"

#include <algorithm>
#include <chrono>
#include <utility>

namespace FujinonSX800 {

namespace {

    bool validConfig(const ScanConfig& config) noexcept
    {
        return config.startAddress > 0U && config.endAddress <= 31U && config.startAddress <= config.endAddress
            && config.timeoutMs > 0U;
    }

} // namespace

BusScanner::BusScanner(std::shared_ptr<ITransport> transport)
    : m_transport { std::move(transport) }
{
}

BusScanner::~BusScanner()
{
    stopScan();
}

void BusScanner::setTransport(std::shared_ptr<ITransport> transport)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state.load() != ScanState::Idle) {
        return;
    }
    m_transport = std::move(transport);
}

std::shared_ptr<ITransport> BusScanner::getTransport() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_transport;
}

bool BusScanner::startScan(const ScanConfig& config)
{
    if (!validConfig(config)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state.load() != ScanState::Idle || !m_transport) {
        return false;
    }

    if (m_worker.joinable()) {
        m_worker.join();
    }
    if (!m_transport->isOpen()) {
        if (!m_transport->open()) {
            return false;
        }
        m_openedTransport = true;
    }

    m_transport->setDataCallback([this](const std::uint8_t* data, std::size_t size) { onDataReceived(data, size); });
    m_stopRequested = false;
    m_pauseRequested = false;
    m_discoveredDevices.clear();
    m_state = ScanState::Scanning;
    m_worker = std::thread(&BusScanner::scanWorker, this, config);
    return true;
}

void BusScanner::stopScan()
{
    m_stopRequested = true;
    m_pauseRequested = false;
    m_responseCv.notify_all();

    if (m_worker.joinable() && std::this_thread::get_id() != m_worker.get_id()) {
        m_worker.join();
    }

    std::shared_ptr<ITransport> transport;
    bool closeTransport { false };
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        transport = m_transport;
        closeTransport = m_openedTransport;
        m_openedTransport = false;
    }
    if (transport) {
        transport->setDataCallback(nullptr);
        if (closeTransport && transport->isOpen()) {
            transport->close();
        }
    }
    setState(ScanState::Idle);
}

void BusScanner::pauseScan()
{
    if (m_state.load() == ScanState::Scanning) {
        m_pauseRequested = true;
        setState(ScanState::Paused);
    }
}

void BusScanner::resumeScan()
{
    if (m_state.load() == ScanState::Paused) {
        m_pauseRequested = false;
        m_responseCv.notify_all();
        setState(ScanState::Scanning);
    }
}

bool BusScanner::isScanning() const noexcept
{
    return m_state.load() == ScanState::Scanning;
}

bool BusScanner::isPaused() const noexcept
{
    return m_state.load() == ScanState::Paused;
}

ScanState BusScanner::getState() const noexcept
{
    return m_state.load();
}

std::vector<DiscoveredDevice> BusScanner::getDiscoveredDevices() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_discoveredDevices;
}

void BusScanner::setDeviceDiscoveredCallback(DeviceDiscoveredCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_deviceCallback = std::move(callback);
}

void BusScanner::setScanProgressCallback(ScanProgressCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_progressCallback = std::move(callback);
}

void BusScanner::setScanStateChangedCallback(ScanStateChangedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stateCallback = std::move(callback);
}

void BusScanner::setScanFinishedCallback(ScanFinishedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_finishedCallback = std::move(callback);
}

void BusScanner::scanWorker(ScanConfig config)
{
    const auto total = static_cast<std::size_t>(config.endAddress - config.startAddress + 1U);
    ProtocolBuilder builder {};

    for (std::uint8_t address = config.startAddress; address <= config.endAddress && !m_stopRequested; ++address) {
        {
            std::unique_lock<std::mutex> lock(m_responseMutex);
            m_responseBuffer.clear();
            m_probeAddress = address;
            m_responseReady = false;
        }

        while (m_pauseRequested && !m_stopRequested) {
            std::unique_lock<std::mutex> lock(m_responseMutex);
            m_responseCv.wait(lock, [this] { return !m_pauseRequested || m_stopRequested; });
        }
        if (m_stopRequested) {
            break;
        }

        builder.setAddress(address);
        const auto query = builder.buildQueryFwVersion();
        const auto started = std::chrono::steady_clock::now();
        const bool sent = m_transport && m_transport->sendData(query);
        bool found { false };
        std::vector<std::uint8_t> response;
        if (sent) {
            std::unique_lock<std::mutex> lock(m_responseMutex);
            found = m_responseCv.wait_for(lock, std::chrono::milliseconds(config.timeoutMs),
                [this] { return m_responseReady || m_stopRequested || m_pauseRequested; });
            if (found && m_responseReady) {
                response = m_responseBuffer;
            }
        }

        if (found && !response.empty()) {
            const auto elapsed
                = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started);
            DiscoveredDevice device;
            device.address = address;
            device.responseTimeMs = static_cast<std::uint32_t>(elapsed.count());
            device.rawResponse = std::move(response);

            DeviceDiscoveredCallback callback;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_discoveredDevices.push_back(device);
                callback = m_deviceCallback;
            }
            if (callback) {
                callback(device);
            }
        }

        ScanProgressCallback progress;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            progress = m_progressCallback;
        }
        if (progress) {
            progress(address, static_cast<std::size_t>(address - config.startAddress + 1U), total);
        }

        if (address != config.endAddress) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.interCommandDelayMs));
        }
    }

    ScanFinishedCallback finished;
    std::vector<DiscoveredDevice> devices;
    std::shared_ptr<ITransport> transport;
    bool closeTransport { false };
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        devices = m_discoveredDevices;
        finished = m_finishedCallback;
        transport = m_transport;
        closeTransport = m_openedTransport;
        m_openedTransport = false;
    }
    if (transport) {
        transport->setDataCallback(nullptr);
        if (closeTransport && transport->isOpen()) {
            transport->close();
        }
    }
    setState(ScanState::Idle);
    if (finished) {
        finished(devices);
    }
}

void BusScanner::onDataReceived(const std::uint8_t* data, std::size_t size)
{
    if (data == nullptr || size == 0U) {
        return;
    }

    const std::vector<std::uint8_t> bytes(data, data + size);
    const auto frames = PelcoDFrame::splitStream(bytes);
    for (const auto& frame : frames) {
        if (!PelcoDFrame::isValidFrame(frame)) {
            continue;
        }
        std::lock_guard<std::mutex> lock(m_responseMutex);
        if (frame.size() >= PelcoDFrame::GeneralResponseSize && frame[1] == m_probeAddress) {
            m_responseBuffer = frame;
            m_responseReady = true;
            m_responseCv.notify_all();
            return;
        }
    }
}

void BusScanner::setState(ScanState state)
{
    m_state = state;
    ScanStateChangedCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        callback = m_stateCallback;
    }
    if (callback) {
        callback(state);
    }
}

} // namespace FujinonSX800

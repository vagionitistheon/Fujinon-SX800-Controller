#include "FujinonCamera.h"
#include "ProtocolParser.h"

#include <chrono>
#include <glog/logging.h>

namespace FujinonSX800 {

FujinonCamera::FujinonCamera(std::shared_ptr<ITransport> transport, std::uint8_t address)
    : m_transport { std::move(transport) }
    , m_builder { address }
    , m_address { address }
{
    m_status.address = address;
}

FujinonCamera::~FujinonCamera()
{
    stop();
}

bool FujinonCamera::start()
{
    if (!m_transport) {
        return false;
    }

    m_transport->setDataCallback([this](const std::vector<std::uint8_t>& data) {
        onDataReceived(data);
    });

    if (!m_transport->isOpen()) {
        if (!m_transport->open()) {
            return false;
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_status.connected = true;
    }

    LOG(INFO) << "Starting FujinonCamera controller (address: " << static_cast<int>(m_address) << ")";

    m_rxRing.clear();
    m_running = true;
    m_rxThread = std::thread(&FujinonCamera::rxLoop, this);
    m_workerThread = std::thread(&FujinonCamera::workerLoop, this);
    m_pollThread = std::thread(&FujinonCamera::pollingLoop, this);

    // Enqueue initial queries
    queryAll();

    return true;
}

void FujinonCamera::stop()
{
    LOG(INFO) << "Stopping FujinonCamera controller";
    m_running = false;
    m_queueCv.notify_all();
    m_rxCv.notify_all();

    if (m_rxThread.joinable()) {
        m_rxThread.join();
    }
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }

    if (m_transport && m_transport->isOpen()) {
        m_transport->close();
    }

    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_status.connected = false;
    }
}

bool FujinonCamera::isConnected() const noexcept
{
    return m_running && m_transport && m_transport->isOpen();
}

void FujinonCamera::setAddress(std::uint8_t address)
{
    m_address = address;
    m_builder.setAddress(address);
    std::lock_guard<std::mutex> lock(m_statusMutex);
    m_status.address = address;
}

std::uint8_t FujinonCamera::getAddress() const noexcept
{
    return m_address;
}

void FujinonCamera::addStatusCallback(StatusCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_statusCallbacks.push_back(std::move(cb));
}

void FujinonCamera::addTrafficCallback(TrafficCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_trafficCallbacks.push_back(std::move(cb));
}

CameraStatus FujinonCamera::getStatus() const
{
    std::lock_guard<std::mutex> lock(m_statusMutex);
    return m_status;
}

void FujinonCamera::enqueueCommand(const std::vector<std::uint8_t>& frame, std::string queryTag)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_commandQueue.push_back({ frame, std::move(queryTag) });
    }
    m_queueCv.notify_one();
}

void FujinonCamera::workerLoop()
{
    while (m_running) {
        CommandItem item;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCv.wait(lock, [this] {
                return !m_commandQueue.empty() || !m_running;
            });

            if (!m_running) {
                break;
            }

            item = std::move(m_commandQueue.front());
            m_commandQueue.pop_front();
        }

        if (m_transport && m_transport->isOpen() && !item.frame.empty()) {
            {
                std::lock_guard<std::mutex> lock(m_statusMutex);
                m_lastQueryTag = item.queryTag;
            }

            VLOG(1) << "Sending command (" << item.frame.size() << " bytes, tag: '" << item.queryTag << "')";
            m_transport->sendData(item.frame);

            // Notify TX callbacks
            std::vector<TrafficCallback> tbs;
            {
                std::lock_guard<std::mutex> lock(m_callbackMutex);
                tbs = m_trafficCallbacks;
            }
            for (const auto& cb : tbs) {
                if (cb) {
                    cb(true, item.frame);
                }
            }
        }

        // 20ms inter-command pacing delay per Pelco-D spec
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void FujinonCamera::pollingLoop()
{
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (!m_running) {
            break;
        }

        // Periodically refresh dynamic telemetry
        enqueueCommand(m_builder.buildQueryZoomPosition());
        enqueueCommand(m_builder.buildQueryFocusPosition());
        enqueueCommand(m_builder.buildQueryLensStatus());
    }
}

void FujinonCamera::onDataReceived(const std::vector<std::uint8_t>& data)
{
    if (!data.empty()) {
        VLOG(2) << "Writing " << data.size() << " bytes to RX ring buffer";
        m_rxRing.writeExact(data.data(), data.size());
        m_rxCv.notify_one();
    }
}

void FujinonCamera::rxLoop()
{
    while (m_running) {
        {
            std::unique_lock<std::mutex> lock(m_rxMutex);
            m_rxCv.wait_for(lock, std::chrono::milliseconds(50), [this] {
                return m_rxRing.availableRead() >= PelcoDFrame::GeneralResponseSize || !m_running;
            });
        }

        if (!m_running) {
            break;
        }

        while (m_running) {
            const std::size_t syncOffset = m_rxRing.findByte(PelcoDFrame::SyncByte);
            if (syncOffset == decltype(m_rxRing)::npos) {
                m_rxRing.advanceRead(m_rxRing.availableRead());
                break;
            }

            if (syncOffset > 0) {
                m_rxRing.advanceRead(syncOffset);
            }

            const std::size_t available = m_rxRing.availableRead();
            if (available < PelcoDFrame::GeneralResponseSize) {
                break;
            }

            constexpr std::size_t candidateSizes[] = {
                PelcoDFrame::QueryResponseSize,
                PelcoDFrame::StandardFrameSize,
                PelcoDFrame::GeneralResponseSize
            };

            bool frameExtracted = false;
            std::array<std::uint8_t, PelcoDFrame::QueryResponseSize> peekBuf {};

            for (const std::size_t candidateSize : candidateSizes) {
                if (available >= candidateSize) {
                    if (m_rxRing.peekBytes(peekBuf.data(), candidateSize)) {
                        const std::uint8_t expectedCksm = peekBuf[candidateSize - 1U];
                        const std::uint8_t computedCksm =
                            PelcoDFrame::calculateChecksum(&peekBuf[1], candidateSize - 2U);

                        if (expectedCksm == computedCksm) {
                            std::vector<std::uint8_t> frame(peekBuf.begin(), peekBuf.begin() + candidateSize);
                            m_rxRing.advanceRead(candidateSize);
                            frameExtracted = true;
                            dispatchFrame(frame);
                            break;
                        }
                    }
                }
            }

            if (!frameExtracted) {
                if (available < PelcoDFrame::QueryResponseSize) {
                    break;
                }
                m_rxRing.advanceRead(1);
            }
        }
    }
}

void FujinonCamera::dispatchFrame(const std::vector<std::uint8_t>& frame)
{
    std::vector<TrafficCallback> tbs;
    std::vector<StatusCallback> sbs;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        tbs = m_trafficCallbacks;
        sbs = m_statusCallbacks;
    }

    for (const auto& cb : tbs) {
        if (cb) {
            cb(false, frame);
        }
    }

    CameraStatus updatedStatus;
    std::string qTag;
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        qTag = m_lastQueryTag;
        updatedStatus = m_status;
    }

    if (ProtocolParser::parsePacket(frame, updatedStatus, qTag)) {
        VLOG(1) << "Packet parsed successfully, updating status (tag: '" << qTag << "')";
        {
            std::lock_guard<std::mutex> lock(m_statusMutex);
            m_status = updatedStatus;
        }
        for (const auto& cb : sbs) {
            if (cb) {
                cb(updatedStatus);
            }
        }
    }
}

void FujinonCamera::zoomTele()
{
    enqueueCommand(m_builder.buildZoom(ZoomDirection::Tele));
}

void FujinonCamera::zoomWide()
{
    enqueueCommand(m_builder.buildZoom(ZoomDirection::Wide));
}

void FujinonCamera::zoomStop()
{
    enqueueCommand(m_builder.buildStop());
}

void FujinonCamera::focusNear()
{
    enqueueCommand(m_builder.buildFocus(FocusDirection::Near));
}

void FujinonCamera::focusFar()
{
    enqueueCommand(m_builder.buildFocus(FocusDirection::Far));
}

void FujinonCamera::focusStop()
{
    enqueueCommand(m_builder.buildStop());
}

void FujinonCamera::irisOpen()
{
    enqueueCommand(m_builder.buildIris(IrisDirection::Open));
}

void FujinonCamera::irisClose()
{
    enqueueCommand(m_builder.buildIris(IrisDirection::Close));
}

void FujinonCamera::irisStop()
{
    enqueueCommand(m_builder.buildStop());
}

void FujinonCamera::setZoomPosition(std::uint16_t pulse)
{
    enqueueCommand(m_builder.buildSetZoomPosition(pulse));
}

void FujinonCamera::setFocusPosition(std::uint16_t pulse)
{
    enqueueCommand(m_builder.buildSetFocusPosition(pulse));
}

void FujinonCamera::setIrisPosition(std::uint16_t pos)
{
    enqueueCommand(m_builder.buildSetIrisPosition(pos));
}

void FujinonCamera::setOnePushAf()
{
    enqueueCommand(m_builder.buildOnePushAf());
}

void FujinonCamera::setAutoFocus(AutoFocusMode mode)
{
    enqueueCommand(m_builder.buildSetAutoFocus(mode));
}

void FujinonCamera::setAfArea(AfArea area)
{
    enqueueCommand(m_builder.buildSetAfArea(area));
}

void FujinonCamera::setAfSensitivity(AfSensitivity sens)
{
    enqueueCommand(m_builder.buildSetAfSensitivity(sens));
}

void FujinonCamera::setDayNightMode(DayNightMode mode)
{
    enqueueCommand(m_builder.buildSetDayNight(mode));
}

void FujinonCamera::setVlcFilter(VlcFilterMode mode)
{
    enqueueCommand(m_builder.buildSetVlcFilter(mode));
}

void FujinonCamera::setDefog(DefogMode mode)
{
    enqueueCommand(m_builder.buildSetDefog(mode));
}

void FujinonCamera::setDeHeatHaze(DeHeatHazeMode mode)
{
    enqueueCommand(m_builder.buildSetDeHeatHaze(mode));
}

void FujinonCamera::setOpticalStabilization(OpticalStabilization mode)
{
    enqueueCommand(m_builder.buildSetOpticalStabilization(mode));
}

void FujinonCamera::setBrightness(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetBrightness(val));
}

void FujinonCamera::setContrast(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetContrast(val));
}

void FujinonCamera::setSaturation(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetSaturation(val));
}

void FujinonCamera::setSharpness(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetSharpness(val));
}

void FujinonCamera::setWhiteBalance(WhiteBalanceMode mode)
{
    enqueueCommand(m_builder.buildSetWhiteBalance(mode));
}

void FujinonCamera::setColorTemperature(std::uint16_t kelvin)
{
    enqueueCommand(m_builder.buildSetColorTemperature(kelvin));
}

void FujinonCamera::setDigitalZoom(DigitalZoomMode mode, std::uint8_t mag)
{
    enqueueCommand(m_builder.buildSetDigitalZoom(mode, mag));
}

void FujinonCamera::setNoiseReduction(NoiseReductionLevel level)
{
    enqueueCommand(m_builder.buildSetNoiseReduction(level));
}

void FujinonCamera::setOpticalFilterDay(OpticalFilter filter)
{
    enqueueCommand(m_builder.buildSetOpticalFilterDay(filter));
}

void FujinonCamera::setOpticalFilterNight(OpticalFilter filter)
{
    enqueueCommand(m_builder.buildSetOpticalFilterNight(filter));
}

void FujinonCamera::setIrWavelength(IrWavelength wl)
{
    enqueueCommand(m_builder.buildSetIrWavelength(wl));
}

void FujinonCamera::setTermination(bool enable)
{
    enqueueCommand(m_builder.buildSetTermination(enable));
}

void FujinonCamera::reboot()
{
    enqueueCommand(m_builder.buildReboot());
}

void FujinonCamera::factoryReset()
{
    enqueueCommand(m_builder.buildFactoryReset());
}

void FujinonCamera::sendMenuKey(MenuKey key)
{
    enqueueCommand(m_builder.buildMenuKey(key));
}

void FujinonCamera::sendRawFrame(const std::vector<std::uint8_t>& frame)
{
    enqueueCommand(frame);
}

void FujinonCamera::queryAll()
{
    enqueueCommand(m_builder.buildQuerySerialNumber(), "QuerySerial");
    enqueueCommand(m_builder.buildQueryFwVersion(), "QueryFw");
    enqueueCommand(m_builder.buildQueryLensStatus(), "QueryLens");
    enqueueCommand(m_builder.buildQueryZoomPosition(), "QueryZoom");
    enqueueCommand(m_builder.buildQueryFocusPosition(), "QueryFocus");
    enqueueCommand(m_builder.buildQueryPhotoSettings(), "QueryPhotoSettings");
    enqueueCommand(m_builder.buildQueryImageQuality(), "QueryImageQuality");
    enqueueCommand(m_builder.buildQueryDisplaySettings(), "QueryDisplaySettings");
    enqueueCommand(m_builder.buildQueryOperationSettings(), "QueryOperationSettings");
    enqueueCommand(m_builder.buildQueryFineSettings(), "QueryFineSettings");
    enqueueCommand(m_builder.buildQueryDayNightEx(), "QueryDayNightEx");
    enqueueCommand(m_builder.buildQuerySpeedEx(), "QuerySpeedEx");
}

} // namespace FujinonSX800

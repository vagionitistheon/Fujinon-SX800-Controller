/// @file TestBusScanner.cpp
/// @brief Tests for asynchronous Pelco-D bus address discovery.

#include "FujinonSX800Core/BusScanner.h"
#include "FujinonSX800Core/PelcoDFrame.h"
#include "TestHelper.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace {

class ScannerTransport final : public FujinonSX800::ITransport {
public:
    explicit ScannerTransport(std::uint8_t deviceAddress)
        : m_deviceAddress { deviceAddress }
    {
    }

    [[nodiscard]] bool open() override
    {
        m_open = true;
        return true;
    }

    void close() override
    {
        m_open = false;
    }

    [[nodiscard]] bool isOpen() const noexcept override
    {
        return m_open.load();
    }

    [[nodiscard]] bool sendData(const std::vector<std::uint8_t>& data) override
    {
        if (!m_open || data.size() != FujinonSX800::PelcoDFrame::StandardFrameSize || data[3] != 0x8BU) {
            return false;
        }
        if (data[1] == m_deviceAddress && m_dataCallback) {
            const auto response = FujinonSX800::PelcoDFrame::createFrame(m_deviceAddress, 0x00U, 0x8BU, 2U, 12U);
            m_dataCallback(response.data(), response.size());
        }
        return true;
    }

    void setDataCallback(DataReceivedCallback callback) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_dataCallback = std::move(callback);
    }

    void setStateCallback(StateChangedCallback callback) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stateCallback = std::move(callback);
    }

private:
    std::uint8_t m_deviceAddress { 1U };
    std::atomic<bool> m_open { false };
    std::mutex m_mutex;
    DataReceivedCallback m_dataCallback;
    StateChangedCallback m_stateCallback;
};

void testDiscoversDevice()
{
    auto transport = std::make_shared<ScannerTransport>(3U);
    FujinonSX800::BusScanner scanner(transport);

    std::mutex mutex;
    std::condition_variable finishedCv;
    bool finished { false };
    std::size_t progressCount { 0U };
    scanner.setScanProgressCallback([&](std::uint8_t, std::size_t, std::size_t) {
        std::lock_guard<std::mutex> lock(mutex);
        ++progressCount;
    });
    scanner.setScanFinishedCallback([&](const std::vector<FujinonSX800::DiscoveredDevice>&) {
        std::lock_guard<std::mutex> lock(mutex);
        finished = true;
        finishedCv.notify_one();
    });

    FujinonSX800::ScanConfig config;
    config.startAddress = 1U;
    config.endAddress = 4U;
    config.timeoutMs = 20U;
    config.interCommandDelayMs = 1U;
    SX800_TEST_ASSERT(scanner.startScan(config));

    std::unique_lock<std::mutex> lock(mutex);
    SX800_TEST_ASSERT(finishedCv.wait_for(lock, std::chrono::seconds(2), [&] { return finished; }));
    const auto devices = scanner.getDiscoveredDevices();
    SX800_TEST_ASSERT(devices.size() == 1U);
    SX800_TEST_ASSERT(devices[0].address == 3U);
    SX800_TEST_ASSERT(devices[0].rawResponse.size() == FujinonSX800::PelcoDFrame::StandardFrameSize);
    SX800_TEST_ASSERT(progressCount == 4U);
}

void testRejectsInvalidRange()
{
    auto transport = std::make_shared<ScannerTransport>(1U);
    FujinonSX800::BusScanner scanner(transport);
    FujinonSX800::ScanConfig config;
    config.startAddress = 0U;
    SX800_TEST_ASSERT(!scanner.startScan(config));
    config.startAddress = 1U;
    config.endAddress = 32U;
    SX800_TEST_ASSERT(!scanner.startScan(config));
}

} // namespace

int main()
{
    testDiscoversDevice();
    testRejectsInvalidRange();
    std::cout << "[PASS] TestBusScanner completed successfully." << std::endl;
    return 0;
}

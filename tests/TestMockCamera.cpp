/// @file TestMockCamera.cpp
/// @brief Integration test of FujinonCamera controller running against MockCameraDevice.

#include "FujinonSX800Core/FujinonCamera.h"
#include "FujinonSX800Core/MockCameraDevice.h"
#include "TestHelper.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

void testMockIntegration()
{
    auto mockDevice = std::make_shared<FujinonSX800::MockCameraDevice>(0x07U);
    FujinonSX800::FujinonCamera camera(mockDevice, 0x07U);

    std::atomic<bool> gotStatusUpdate { false };
    camera.addStatusCallback([&](const FujinonSX800::CameraStatus& status) {
        if (!status.serialNumber.empty() && status.fwVersionMajor == 2U) {
            gotStatusUpdate = true;
        }
    });

    camera.setAutoQueryOnConnect(true);
    const bool started = camera.start();
    SX800_TEST_ASSERT(started);
    SX800_TEST_ASSERT(camera.isConnected());

    // Allow worker to query initial status
    for (int i = 0; i < 50; ++i) {
        if (gotStatusUpdate) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    SX800_TEST_ASSERT(gotStatusUpdate);
    const auto status = camera.getStatus();
    SX800_TEST_ASSERT(status.serialNumber == "SX800999");
    SX800_TEST_ASSERT(status.fwVersionMajor == 2U && status.fwVersionMinor == 12U);
    SX800_TEST_ASSERT(status.rs485Address == 0x07U);

    // Test commanding zoom position
    camera.setZoomPosition(12345U);
    bool zoomApplied { false };
    for (int i { 0 }; i < 50; ++i) {
        if (mockDevice->getInternalState().zoomPosition == 12345U) {
            zoomApplied = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    SX800_TEST_ASSERT(zoomApplied);

    // Test setting defog mode
    camera.setDefog(FujinonSX800::DefogMode::High);
    bool defogApplied { false };
    for (int i { 0 }; i < 50; ++i) {
        if (mockDevice->getInternalState().defogMode == FujinonSX800::DefogMode::High) {
            defogApplied = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    SX800_TEST_ASSERT(defogApplied);

    // Test query response timeout
    std::atomic<bool> timeoutTriggered { false };
    std::string timedOutTag;
    camera.setQueryTimeoutMs(50U);
    camera.addTimeoutCallback([&](const std::string& tag) {
        timeoutTriggered = true;
        timedOutTag = tag;
    });

    // Send an unhandled command with a query tag to trigger timeout (address 0x09 is ignored by mock device)
    camera.sendQueryFrame(FujinonSX800::PelcoDFrame::createFrame(0x09U, 0x00U, 0x55U, 0x00U, 0x00U), "TestQuery");
    for (int i = 0; i < 40; ++i) {
        if (timeoutTriggered.load()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    SX800_TEST_ASSERT(timeoutTriggered.load());
    SX800_TEST_ASSERT(timedOutTag == "TestQuery");

    camera.stop();
    SX800_TEST_ASSERT(!camera.isConnected());
}

class FragmentedMockTransport : public FujinonSX800::ITransport {
public:
    explicit FragmentedMockTransport(std::uint8_t address = 7U)
        : m_address { address }
    {
    }

    bool open() override
    {
        m_open = true;
        return true;
    }

    void close() override
    {
        m_open = false;
    }

    bool isOpen() const noexcept override
    {
        return m_open.load();
    }

    bool sendData(const std::vector<std::uint8_t>& data) override
    {
        if (data.size() == 7U && data[2] == 0x00U && (data[3] == 0x89U || data[3] == 0x7BU)) {
            // Craft 18-byte response where the first 7 bytes collide with a valid 7-byte checksum:
            // sum(0x07, 'S'(83), 'X'(88), '8'(56), 'c'(99)) == 333 == 256 + 77 ('M')
            // Byte 6 is 'M' (0x4D), matching the 7-byte checksum while being printable ASCII.
            std::vector<std::uint8_t> fullFrame(18U, 0x00U);
            fullFrame[0] = FujinonSX800::PelcoDFrame::SyncByte;
            fullFrame[1] = m_address;
            fullFrame[2] = 'S';
            fullFrame[3] = 'X';
            fullFrame[4] = '8';
            fullFrame[5] = 'c';
            fullFrame[6] = 'M';
            fullFrame[7] = '0';
            fullFrame[8] = '1';
            fullFrame[9] = '2';
            fullFrame[17] = FujinonSX800::PelcoDFrame::calculateChecksum(&fullFrame[1], 16U);

            // Send fragment 1: first 7 bytes
            if (m_dataCallback) {
                m_dataCallback(fullFrame.data(), 7U);
            }

            // Small delay to simulate packet fragmentation across network segments
            std::this_thread::sleep_for(std::chrono::milliseconds(15));

            // Send fragment 2: remaining 11 bytes
            if (m_dataCallback) {
                m_dataCallback(fullFrame.data() + 7U, 11U);
            }
        }
        return true;
    }

    void setDataCallback(DataReceivedCallback callback) override
    {
        m_dataCallback = std::move(callback);
    }

    void setStateCallback(StateChangedCallback callback) override
    {
        m_stateCallback = std::move(callback);
    }

private:
    std::uint8_t m_address { 7U };
    std::atomic<bool> m_open { false };
    DataReceivedCallback m_dataCallback;
    StateChangedCallback m_stateCallback;
};

void testFragmentedSerialResponse()
{
    auto mockTransport = std::make_shared<FragmentedMockTransport>(0x07U);
    FujinonSX800::FujinonCamera camera(mockTransport, 0x07U);

    std::atomic<bool> gotSerial { false };
    camera.addStatusCallback([&](const FujinonSX800::CameraStatus& status) {
        if (!status.serialNumber.empty()) {
            gotSerial = true;
        }
    });

    const bool started = camera.start();
    SX800_TEST_ASSERT(started);

    FujinonSX800::ProtocolBuilder builder(0x07U);
    camera.sendQueryFrame(builder.buildQuerySerialNumber(), "QuerySerial");

    for (int i = 0; i < 50; ++i) {
        if (gotSerial.load()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    SX800_TEST_ASSERT(gotSerial.load());
    SX800_TEST_ASSERT(camera.getStatus().serialNumber.rfind("SX8c", 0) == 0);

    camera.stop();
}

void testRxOverflowTracking()
{
    class OverflowTransport : public FujinonSX800::ITransport {
    public:
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
            return m_open;
        }
        [[nodiscard]] bool sendData([[maybe_unused]] const std::vector<std::uint8_t>& data) override
        {
            return true;
        }
        void setDataCallback(DataReceivedCallback callback) override
        {
            m_dataCallback = std::move(callback);
        }
        void setStateCallback(StateChangedCallback callback) override
        {
            m_stateCallback = std::move(callback);
        }
        void triggerData(const std::uint8_t* data, std::size_t size)
        {
            if (m_dataCallback) {
                m_dataCallback(data, size);
            }
        }

    private:
        bool m_open { false };
        DataReceivedCallback m_dataCallback;
        StateChangedCallback m_stateCallback;
    };

    auto transport = std::make_shared<OverflowTransport>();
    FujinonSX800::FujinonCamera camera(transport, 0x07U);
    SX800_TEST_ASSERT(camera.rxOverflowDrops() == 0ULL);

    const bool started = camera.start();
    SX800_TEST_ASSERT(started);

    // Trigger chunk exceeding the 64 KiB ring buffer capacity
    const std::vector<std::uint8_t> hugeChunk(70000U, 0x00U);
    transport->triggerData(hugeChunk.data(), hugeChunk.size());

    SX800_TEST_ASSERT(camera.rxOverflowDrops() == 70000ULL);

    camera.stop();
}

#include <glog/logging.h>

int main([[maybe_unused]] int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;
    FLAGS_colorlogtostderr = true;

    testMockIntegration();
    testFragmentedSerialResponse();
    testRxOverflowTracking();

    std::cout << "[PASS] TestMockCamera completed successfully." << std::endl;
    google::ShutdownGoogleLogging();
    return 0;
}

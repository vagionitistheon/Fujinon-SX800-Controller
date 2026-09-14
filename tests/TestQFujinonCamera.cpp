/// @file TestQFujinonCamera.cpp
/// @brief Qt adapter tests for transport state, configuration, and concurrent status access.

#include "FujinonSX800Core/ITransport.h"
#include "QFujinonCamera.h"
#include "TestHelper.h"

#include <QCoreApplication>
#include <QSignalSpy>

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

class QtStateTransport final : public FujinonSX800::ITransport {
public:
    [[nodiscard]] bool open() override
    {
        m_open = true;
        emitState(FujinonSX800::TransportState::Connected, "connected");
        return true;
    }

    void close() override
    {
        m_open = false;
        emitState(FujinonSX800::TransportState::Disconnected, "disconnected");
    }

    [[nodiscard]] bool isOpen() const noexcept override
    {
        return m_open.load();
    }

    [[nodiscard]] bool sendData(const std::vector<std::uint8_t>&) override
    {
        return m_open.load();
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

    void emitState(FujinonSX800::TransportState state, const std::string& message)
    {
        StateChangedCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            callback = m_stateCallback;
        }
        if (callback) {
            callback(state, message);
        }
    }

    void emitData(const std::vector<std::uint8_t>& data)
    {
        DataReceivedCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            callback = m_dataCallback;
        }
        if (callback) {
            callback(data.data(), data.size());
        }
    }

private:
    std::atomic<bool> m_open { false };
    std::mutex m_mutex;
    DataReceivedCallback m_dataCallback;
    StateChangedCallback m_stateCallback;
};

void testTransportSignalsAndStatus()
{
    auto transport = std::make_shared<QtStateTransport>();
    FujinonSX800Qt::QFujinonCamera camera(transport, 7U);
    QSignalSpy stateSpy(&camera, &FujinonSX800Qt::QFujinonCamera::transportStateChanged);
    QSignalSpy connectionSpy(&camera, &FujinonSX800Qt::QFujinonCamera::connectionStateChanged);

    SX800_TEST_ASSERT(camera.connectCamera());
    QCoreApplication::processEvents();
    SX800_TEST_ASSERT(stateSpy.count() >= 1);
    SX800_TEST_ASSERT(connectionSpy.count() >= 1);
    SX800_TEST_ASSERT(camera.currentStatus().isConnected);

    transport->emitState(FujinonSX800::TransportState::Error, "link failure");
    QCoreApplication::processEvents();
    SX800_TEST_ASSERT(stateSpy.count() >= 2);
    SX800_TEST_ASSERT(!camera.currentStatus().isConnected);
    SX800_TEST_ASSERT(camera.currentStatus().transportState == FujinonSX800::TransportState::Error);
    SX800_TEST_ASSERT(camera.currentStatus().transportError == "link failure");

    camera.disconnectCamera();
}

void testConfigurationOptions()
{
    auto transport = std::make_shared<QtStateTransport>();
    FujinonSX800Qt::QFujinonCamera camera(transport, 1U);

    SX800_TEST_ASSERT(camera.getAutoReconnect());
    camera.setAutoReconnect(false);
    SX800_TEST_ASSERT(!camera.getAutoReconnect());
    camera.setAutoReconnect(true);
    SX800_TEST_ASSERT(camera.getAutoReconnect());

    SX800_TEST_ASSERT(camera.connectCamera());
    camera.setAutoQueryOnConnect(false);
    camera.setTelemetryPolling(true, 500);
    camera.setQueryTimeoutMs(2000);
    camera.setAutoReconnect(false);
    SX800_TEST_ASSERT(!camera.getAutoReconnect());

    camera.disconnectCamera();
}

void testConcurrentStatusAccess()
{
    auto transport = std::make_shared<QtStateTransport>();
    FujinonSX800Qt::QFujinonCamera camera(transport, 1U);
    SX800_TEST_ASSERT(camera.connectCamera());

    std::atomic<bool> runReaders { true };
    std::atomic<std::uint32_t> readOps { 0U };

    // Spawn 4 concurrent readers calling status methods
    std::vector<std::thread> readers;
    readers.reserve(4);
    for (std::size_t i { 0 }; i < 4; ++i) {
        readers.emplace_back([&]() {
            while (runReaders.load(std::memory_order_relaxed)) {
                const auto status = camera.currentStatus();
                const bool connected = camera.isConnected();
                const auto drops = camera.rxOverflowDrops();
                (void)status;
                (void)connected;
                (void)drops;
                readOps.fetch_add(1U, std::memory_order_relaxed);
            }
        });
    }

    // Main thread toggles state in background
    for (std::size_t cycle { 0 }; cycle < 20; ++cycle) {
        transport->emitState(FujinonSX800::TransportState::Connecting, "cycle connect");
        QCoreApplication::processEvents();
        transport->emitState(FujinonSX800::TransportState::Connected, "cycle ok");
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    runReaders.store(false, std::memory_order_relaxed);
    for (auto& reader : readers) {
        reader.join();
    }

    camera.disconnectCamera();
    std::cout << "[PASS] testConcurrentStatusAccess: " << readOps.load() << " reads performed safely." << std::endl;
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    testTransportSignalsAndStatus();
    testConfigurationOptions();
    testConcurrentStatusAccess();
    std::cout << "[PASS] TestQFujinonCamera completed successfully." << std::endl;
    return 0;
}

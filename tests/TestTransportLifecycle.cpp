/// @file TestTransportLifecycle.cpp
/// @brief Lifecycle, error-notification, and concurrency tests for native transports.

#include "FujinonSX800Core/BaseTransport.h"
#include "FujinonSX800Core/SerialTransport.h"
#include "FujinonSX800Core/TcpTransport.h"
#include "FujinonSX800Core/UdpTransport.h"
#include "TestHelper.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

template <typename Transport> void assertErrorOnOpen(Transport& transport, const std::string& expected)
{
    std::atomic<FujinonSX800::TransportState> state { FujinonSX800::TransportState::Disconnected };
    std::string message;
    transport.setStateCallback([&](FujinonSX800::TransportState next, const std::string& detail) {
        state = next;
        message = detail;
    });

    SX800_TEST_ASSERT(!transport.open());
    SX800_TEST_ASSERT(state.load() == FujinonSX800::TransportState::Error);
    SX800_TEST_ASSERT(message.find(expected) != std::string::npos);
    SX800_TEST_ASSERT(!transport.isOpen());
}

void testSerialConfigError()
{
    FujinonSX800::SerialTransport transport;
    assertErrorOnOpen(transport, "Serial port name is empty");
}

void testTcpConfigError()
{
    FujinonSX800::TcpTransport emptyHost("", 4001U);
    assertErrorOnOpen(emptyHost, "Host address is empty");

    FujinonSX800::TcpTransport emptyPort("127.0.0.1", 0U);
    assertErrorOnOpen(emptyPort, "TCP port is zero");
}

void testUdpConfigErrors()
{
    FujinonSX800::UdpTransport emptyHost("", 4001U);
    assertErrorOnOpen(emptyHost, "UDP host is empty");

    FujinonSX800::UdpTransport emptyPort("127.0.0.1", 0U);
    assertErrorOnOpen(emptyPort, "UDP port is zero");
}

void testClosedTransportSafety()
{
    const std::vector<std::uint8_t> dummyData { 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01 };

    FujinonSX800::SerialTransport serial("COM_NONEXISTENT", 9600U);
    SX800_TEST_ASSERT(!serial.isOpen());
    SX800_TEST_ASSERT(!serial.sendData(dummyData));
    serial.close();
    SX800_TEST_ASSERT(!serial.isOpen());

    FujinonSX800::TcpTransport tcp("127.0.0.1", 54321U);
    SX800_TEST_ASSERT(!tcp.isOpen());
    SX800_TEST_ASSERT(!tcp.sendData(dummyData));
    tcp.close();
    SX800_TEST_ASSERT(!tcp.isOpen());

    FujinonSX800::UdpTransport udp("127.0.0.1", 54322U);
    SX800_TEST_ASSERT(!udp.isOpen());
    SX800_TEST_ASSERT(!udp.sendData(dummyData));
    udp.close();
    SX800_TEST_ASSERT(!udp.isOpen());
}

class ConcurrentTestTransport final : public FujinonSX800::BaseTransport {
public:
    [[nodiscard]] bool open() override
    {
        m_running.store(true);
        notifyState(FujinonSX800::TransportState::Connected, "open");
        return true;
    }

    void close() override
    {
        m_running.store(false);
        notifyState(FujinonSX800::TransportState::Disconnected, "closed");
    }

    [[nodiscard]] bool isOpen() const noexcept override
    {
        return m_running.load();
    }

    [[nodiscard]] bool sendData(const std::vector<std::uint8_t>&) override
    {
        return m_running.load();
    }

    void fireState(FujinonSX800::TransportState state, const std::string& msg)
    {
        notifyState(state, msg);
    }

    void fireData(const std::uint8_t* data, std::size_t size)
    {
        invokeDataCallback(data, size);
    }
};

void testConcurrentCallbackSafety()
{
    ConcurrentTestTransport transport;
    SX800_TEST_ASSERT(transport.open());

    std::atomic<bool> runThreads { true };
    std::atomic<std::uint32_t> dataEvents { 0U };
    std::atomic<std::uint32_t> stateEvents { 0U };

    // Producer 1: Dispatches data callbacks in loop
    std::thread dataProducer([&]() {
        const std::uint8_t dummyByte { 0x42U };
        while (runThreads.load(std::memory_order_relaxed)) {
            transport.fireData(&dummyByte, 1U);
            std::this_thread::yield();
        }
    });

    // Producer 2: Dispatches state callbacks in loop
    std::thread stateProducer([&]() {
        while (runThreads.load(std::memory_order_relaxed)) {
            transport.fireState(FujinonSX800::TransportState::Connected, "active");
            std::this_thread::yield();
        }
    });

    // Consumer/Mutator: Concurrently overwrites and clears callbacks
    std::thread callbackMutator([&]() {
        while (runThreads.load(std::memory_order_relaxed)) {
            transport.setDataCallback(
                [&](const std::uint8_t*, std::size_t) { dataEvents.fetch_add(1U, std::memory_order_relaxed); });
            transport.setStateCallback([&](FujinonSX800::TransportState, const std::string&) {
                stateEvents.fetch_add(1U, std::memory_order_relaxed);
            });
            std::this_thread::yield();
            transport.setDataCallback(nullptr);
            transport.setStateCallback(nullptr);
        }
    });

    // Allow concurrent threads to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    runThreads.store(false, std::memory_order_relaxed);

    dataProducer.join();
    stateProducer.join();
    callbackMutator.join();

    transport.close();
    SX800_TEST_ASSERT(!transport.isOpen());
    std::cout << "[PASS] testConcurrentCallbackSafety: " << dataEvents.load() << " data events, " << stateEvents.load()
              << " state events handled safely." << std::endl;
}

} // namespace

int main()
{
    testSerialConfigError();
    testTcpConfigError();
    testUdpConfigErrors();
    testClosedTransportSafety();
    testConcurrentCallbackSafety();
    std::cout << "[PASS] TestTransportLifecycle completed successfully." << std::endl;
    return 0;
}

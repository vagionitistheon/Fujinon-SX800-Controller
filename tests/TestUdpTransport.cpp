/// @file TestUdpTransport.cpp
/// @brief Loopback tests for the cross-platform UDP transport.

#include "FujinonSX800Core/UdpTransport.h"
#include "TestHelper.h"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <vector>

namespace {

constexpr std::uint16_t ReceiverPort { 42101U };
constexpr std::uint16_t SenderPort { 42102U };

void testLoopbackDatagram()
{
    FujinonSX800::UdpTransport receiver("127.0.0.1", SenderPort, ReceiverPort);
    FujinonSX800::UdpTransport sender("127.0.0.1", ReceiverPort, SenderPort);

    std::mutex receiveMutex;
    std::condition_variable receiveCv;
    std::vector<std::uint8_t> received;
    receiver.setDataCallback([&](const std::uint8_t* data, std::size_t size) {
        std::lock_guard<std::mutex> lock(receiveMutex);
        received.assign(data, data + size);
        receiveCv.notify_one();
    });

    SX800_TEST_ASSERT(receiver.open());
    SX800_TEST_ASSERT(sender.open());
    SX800_TEST_ASSERT(receiver.isOpen());
    SX800_TEST_ASSERT(sender.isOpen());
    SX800_TEST_ASSERT(receiver.getLocalPort() == ReceiverPort);
    SX800_TEST_ASSERT(sender.getLocalPort() == SenderPort);

    const std::vector<std::uint8_t> payload { 0xFFU, 0x07U, 0x00U, 0x20U, 0x00U, 0x00U, 0x27U };
    SX800_TEST_ASSERT(sender.sendData(payload));

    std::unique_lock<std::mutex> lock(receiveMutex);
    const bool receivedInTime = receiveCv.wait_for(lock, std::chrono::seconds(2), [&] { return received == payload; });
    SX800_TEST_ASSERT(receivedInTime);

    sender.close();
    receiver.close();
    SX800_TEST_ASSERT(!sender.isOpen());
    SX800_TEST_ASSERT(!receiver.isOpen());
}

void testInvalidConfiguration()
{
    FujinonSX800::UdpTransport emptyHost("", ReceiverPort);
    SX800_TEST_ASSERT(!emptyHost.open());

    FujinonSX800::UdpTransport emptyPort("127.0.0.1", 0U);
    SX800_TEST_ASSERT(!emptyPort.open());
}

} // namespace

int main()
{
    testLoopbackDatagram();
    testInvalidConfiguration();
    std::cout << "[PASS] TestUdpTransport completed successfully." << std::endl;
    return 0;
}

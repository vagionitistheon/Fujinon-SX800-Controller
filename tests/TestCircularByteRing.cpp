/**
 * @file TestCircularByteRing.cpp
 * @brief Unit and concurrency tests for SPSC CircularByteRing.
 */

#include "CircularByteRing.h"
#include "TestHelper.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace FujinonSX800;

static void testBasicOperations()
{
    CircularByteRing<1024> ring;
    SX800_TEST_ASSERT(ring.capacity() == 1024);
    SX800_TEST_ASSERT(ring.availableRead() == 0);
    SX800_TEST_ASSERT(ring.availableWrite() == 1024);

    const std::vector<std::uint8_t> testData = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };
    const bool writeOk = ring.writeExact(testData.data(), testData.size());
    SX800_TEST_ASSERT(writeOk);
    SX800_TEST_ASSERT(ring.availableRead() == 5);
    SX800_TEST_ASSERT(ring.availableWrite() == 1019);

    std::uint8_t peekBuf[5] = {};
    const bool peekOk = ring.peekBytes(peekBuf, 5);
    SX800_TEST_ASSERT(peekOk);
    for (std::size_t i = 0; i < 5; ++i) {
        SX800_TEST_ASSERT(peekBuf[i] == testData[i]);
    }
    SX800_TEST_ASSERT(ring.availableRead() == 5); // Read position unchanged

    std::uint8_t readBuf[5] = {};
    const bool readOk = ring.readExact(readBuf, 5);
    SX800_TEST_ASSERT(readOk);
    for (std::size_t i = 0; i < 5; ++i) {
        SX800_TEST_ASSERT(readBuf[i] == testData[i]);
    }
    SX800_TEST_ASSERT(ring.availableRead() == 0);
    std::cout << "[PASS] testBasicOperations\n";
}

static void testWrapAround()
{
    CircularByteRing<16> ring; // Small ring to test wrapping quickly

    // Write 12 bytes
    std::vector<std::uint8_t> chunk1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    const bool write1Ok = ring.writeExact(chunk1.data(), chunk1.size());
    SX800_TEST_ASSERT(write1Ok);

    // Read 8 bytes (advancing head to index 8)
    std::uint8_t readBuf[8] = {};
    const bool read1Ok = ring.readExact(readBuf, 8);
    SX800_TEST_ASSERT(read1Ok);
    SX800_TEST_ASSERT(ring.availableRead() == 4);

    // Now write 10 bytes -> will wrap around index 16
    std::vector<std::uint8_t> chunk2 = { 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 };
    const bool write2Ok = ring.writeExact(chunk2.data(), chunk2.size());
    SX800_TEST_ASSERT(write2Ok);
    SX800_TEST_ASSERT(ring.availableRead() == 14);

    // Verify ReadView spans both contiguous regions
    const auto readView = ring.getReadView();
    SX800_TEST_ASSERT(readView.first.size > 0);
    SX800_TEST_ASSERT(readView.second.size > 0);
    SX800_TEST_ASSERT(readView.totalSize() == 14);

    // Read all 14 bytes and verify content
    std::uint8_t allBuf[14] = {};
    const bool readAllOk = ring.readExact(allBuf, 14);
    SX800_TEST_ASSERT(readAllOk);
    SX800_TEST_ASSERT(allBuf[0] == 9);
    SX800_TEST_ASSERT(allBuf[3] == 12);
    SX800_TEST_ASSERT(allBuf[4] == 13);
    SX800_TEST_ASSERT(allBuf[13] == 22);

    SX800_TEST_ASSERT(ring.availableRead() == 0);
    std::cout << "[PASS] testWrapAround\n";
}

static void testFindByte()
{
    CircularByteRing<32> ring;

    // Fill partially, consume partially to position head near end
    std::vector<std::uint8_t> pad(24, 0x00);
    const bool writePadOk = ring.writeExact(pad.data(), pad.size());
    SX800_TEST_ASSERT(writePadOk);
    ring.advanceRead(24);

    // Write bytes wrapping around: 0x01, 0x02, 0xFF (delimiter!), 0x03
    std::vector<std::uint8_t> data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xFF, 0x09, 0x0A };
    const bool writeDataOk = ring.writeExact(data.data(), data.size());
    SX800_TEST_ASSERT(writeDataOk);

    // Find 0xFF
    const std::size_t foundIdx = ring.findByte(0xFF);
    SX800_TEST_ASSERT(foundIdx == 8);

    // Find non-existent
    const std::size_t missingIdx = ring.findByte(0xEE);
    SX800_TEST_ASSERT(missingIdx == decltype(ring)::npos);

    std::cout << "[PASS] testFindByte\n";
}

static void testConcurrentSpsc()
{
    constexpr std::size_t TotalBytes = 2 * 1024 * 1024; // 2 MB
    CircularByteRing<65536> ring;
    std::atomic<bool> producerDone { false };

    // Producer Thread
    std::thread producer([&ring, &producerDone]() {
        std::size_t bytesWritten = 0;
        std::uint8_t val = 0;
        std::vector<std::uint8_t> chunk(128);

        while (bytesWritten < TotalBytes) {
            const std::size_t toWrite = std::min<std::size_t>(chunk.size(), TotalBytes - bytesWritten);
            for (std::size_t i = 0; i < toWrite; ++i) {
                chunk[i] = val++;
            }

            while (!ring.writeExact(chunk.data(), toWrite)) {
                std::this_thread::yield();
            }
            bytesWritten += toWrite;
        }
        producerDone.store(true, std::memory_order_release);
    });

    // Consumer Thread
    std::size_t bytesRead = 0;
    std::uint8_t expectedVal = 0;
    std::vector<std::uint8_t> recvBuf(256);

    while (bytesRead < TotalBytes) {
        const std::size_t avail = ring.availableRead();
        if (avail == 0) {
            if (producerDone.load(std::memory_order_acquire) && ring.availableRead() == 0) {
                break;
            }
            std::this_thread::yield();
            continue;
        }

        const std::size_t toRead = std::min<std::size_t>(avail, recvBuf.size());
        if (ring.readExact(recvBuf.data(), toRead)) {
            for (std::size_t i = 0; i < toRead; ++i) {
                SX800_TEST_ASSERT(recvBuf[i] == expectedVal);
                ++expectedVal;
            }
            bytesRead += toRead;
        }
    }

    producer.join();
    SX800_TEST_ASSERT(bytesRead == TotalBytes);
    SX800_TEST_ASSERT(ring.availableRead() == 0);
    std::cout << "[PASS] testConcurrentSpsc (" << (TotalBytes / (1024 * 1024)) << " MB verified)\n";
}

int main()
{
    std::cout << "Starting SPSC CircularByteRing Tests...\n";
    testBasicOperations();
    testWrapAround();
    testFindByte();
    testConcurrentSpsc();
    std::cout << "All SPSC CircularByteRing Tests PASSED!\n";
    return 0;
}

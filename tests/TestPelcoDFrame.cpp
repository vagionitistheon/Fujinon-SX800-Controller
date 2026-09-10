/// @file TestPelcoDFrame.cpp
/// @brief Unit tests for Pelco-D checksum and stream framing.

#include "FujinonSX800Core/PelcoDFrame.h"

#include <cassert>
#include <iostream>
#include <vector>

void testChecksum()
{
    // Test known Pelco-D vector: [Addr=0x01, Cmd1=0x00, Cmd2=0x20, Data1=0x00, Data2=0x00] -> Sum = 0x21
    const std::vector<std::uint8_t> payload { 0x01U, 0x00U, 0x20U, 0x00U, 0x00U };
    const std::uint8_t cksm = FujinonSX800::PelcoDFrame::calculateChecksum(payload);
    assert(cksm == 0x21U);

    // Overflow test (modulo 256)
    const std::vector<std::uint8_t> overflowPayload { 0xFFU, 0x02U, 0x00U, 0x00U, 0x00U };
    assert(FujinonSX800::PelcoDFrame::calculateChecksum(overflowPayload) == 0x01U);
}

void testValidFrames()
{
    // 4-byte general response
    const std::vector<std::uint8_t> valid4 { 0xFFU, 0x07U, 0x00U, 0x07U };
    assert(FujinonSX800::PelcoDFrame::isValidFrame(valid4));

    const std::vector<std::uint8_t> invalid4 { 0xFFU, 0x07U, 0x00U, 0x08U };
    assert(!FujinonSX800::PelcoDFrame::isValidFrame(invalid4));

    // 7-byte extended frame
    const auto valid7 = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x20U, 0x00U, 0x00U);
    assert(valid7.size() == 7U);
    assert(FujinonSX800::PelcoDFrame::isValidFrame(valid7));

    // 18-byte query frame
    std::vector<std::uint8_t> valid18(18U, 0x00U);
    valid18[0] = 0xFFU;
    valid18[1] = 0x07U; // Addr
    valid18[2] = 'S';
    valid18[3] = 'X';
    valid18[4] = '8';
    valid18[5] = '0';
    valid18[6] = '0';
    valid18[17] = FujinonSX800::PelcoDFrame::calculateChecksum(&valid18[1], 16U);
    assert(FujinonSX800::PelcoDFrame::isValidFrame(valid18));
}

void testStreamSplitting()
{
    // Stream with junk prefix, a 7-byte frame, junk middle, a 4-byte frame, and trailing bytes
    std::vector<std::uint8_t> stream { 0x12U, 0x34U };

    const auto frame1 = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x4FU, 0x12U, 0x34U);
    stream.insert(stream.end(), frame1.begin(), frame1.end());

    stream.push_back(0xAAU);

    const std::vector<std::uint8_t> frame2 { 0xFFU, 0x07U, 0x00U, 0x07U };
    stream.insert(stream.end(), frame2.begin(), frame2.end());

    stream.push_back(0xBBU);

    const auto split = FujinonSX800::PelcoDFrame::splitStream(stream);
    assert(split.size() == 2U);
    assert(split[0] == frame1);
    assert(split[1] == frame2);
}

int main()
{
    testChecksum();
    testValidFrames();
    testStreamSplitting();

    std::cout << "[PASS] TestPelcoDFrame completed successfully." << std::endl;
    return 0;
}

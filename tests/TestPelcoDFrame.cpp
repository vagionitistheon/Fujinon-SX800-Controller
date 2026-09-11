/// @file TestPelcoDFrame.cpp
/// @brief Unit tests for Pelco-D checksum and stream framing.

#include "FujinonSX800Core/PelcoDFrame.h"
#include "TestHelper.h"

#include <iostream>
#include <vector>

void testChecksum()
{
    // Test known Pelco-D vector: [Addr=0x01, Cmd1=0x00, Cmd2=0x20, Data1=0x00, Data2=0x00] -> Sum = 0x21
    const std::vector<std::uint8_t> payload { 0x01U, 0x00U, 0x20U, 0x00U, 0x00U };
    const std::uint8_t cksm = FujinonSX800::PelcoDFrame::calculateChecksum(payload);
    SX800_TEST_ASSERT(cksm == 0x21U);

    // Overflow test (modulo 256)
    const std::vector<std::uint8_t> overflowPayload { 0xFFU, 0x02U, 0x00U, 0x00U, 0x00U };
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::calculateChecksum(overflowPayload) == 0x01U);
}

void testValidFrames()
{
    // 4-byte general response
    const std::vector<std::uint8_t> valid4 { 0xFFU, 0x07U, 0x00U, 0x07U };
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::isValidFrame(valid4));

    const std::vector<std::uint8_t> invalid4 { 0xFFU, 0x07U, 0x00U, 0x08U };
    SX800_TEST_ASSERT(!FujinonSX800::PelcoDFrame::isValidFrame(invalid4));

    // 4-byte checksum alias: byte[1] + byte[2] == checksum, but byte[2] != 0x00
    const std::vector<std::uint8_t> aliased4 { 0xFFU, 0x07U, 0x05U, 0x0CU };
    SX800_TEST_ASSERT(!FujinonSX800::PelcoDFrame::isValidFrame(aliased4));

    // 7-byte extended frame
    const auto valid7 = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x20U, 0x00U, 0x00U);
    SX800_TEST_ASSERT(valid7.size() == 7U);
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::isValidFrame(valid7));

    // 7-byte checksum alias: valid checksum over 5 bytes, but byte[2] is not a valid opcode (e.g., 'S' = 0x53)
    const std::vector<std::uint8_t> aliased7 { 0xFFU, 0x07U, 0x53U, 0x58U, 0x38U, 0x30U, 0x1AU };
    SX800_TEST_ASSERT(!FujinonSX800::PelcoDFrame::isValidFrame(aliased7));

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
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::isValidFrame(valid18));
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
    SX800_TEST_ASSERT(split.size() == 2U);
    SX800_TEST_ASSERT(split[0] == frame1);
    SX800_TEST_ASSERT(split[1] == frame2);

    // Stream with aliased 7-byte fragment where checksum matches but byte[2] is not an opcode
    const std::vector<std::uint8_t> aliasedFragmentStream { 0xFFU, 0x07U, 0x53U, 0x58U, 0x38U, 0x30U, 0x1AU };
    const auto splitAliased = FujinonSX800::PelcoDFrame::splitStream(aliasedFragmentStream);
    SX800_TEST_ASSERT(splitAliased.empty());
}

int main()
{
    testChecksum();
    testValidFrames();
    testStreamSplitting();

    std::cout << "[PASS] TestPelcoDFrame completed successfully." << std::endl;
    return 0;
}

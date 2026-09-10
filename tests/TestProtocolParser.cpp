/// @file TestProtocolParser.cpp
/// @brief Unit tests for Pelco-D response decoding and telemetry population.

#include "FujinonSX800Core/PelcoDFrame.h"
#include "FujinonSX800Core/ProtocolParser.h"

#include <cassert>
#include <iostream>

void testGeneralAck()
{
    FujinonSX800::CameraStatus status;
    const std::vector<std::uint8_t> ack { 0xFFU, 0x07U, 0x00U, 0x07U };
    assert(FujinonSX800::ProtocolParser::parsePacket(ack, status));
}

void testExtendedResponses()
{
    FujinonSX800::CameraStatus status;

    // Zoom Position Response: 0x00, 0x5D, 0x12, 0x34
    const auto zoomResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x5DU, 0x12U, 0x34U);
    assert(FujinonSX800::ProtocolParser::parsePacket(zoomResp, status));
    assert(status.zoomPosition == 0x1234U);
    assert(status.focalLengthMm > 20.0);

    // Focus Position Response: 0x00, 0x81, 0x20, 0x00
    const auto focusResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x81U, 0x20U, 0x00U);
    assert(FujinonSX800::ProtocolParser::parsePacket(focusResp, status));
    assert(status.focusPosition == 0x2000U);

    // FW Version Response: 0x00, 0x8B, 0x02, 0x0C (v2.12)
    const auto fwResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x8BU, 0x02U, 0x0CU);
    assert(FujinonSX800::ProtocolParser::parsePacket(fwResp, status));
    assert(status.fwVersionMajor == 2U && status.fwVersionMinor == 12U);

    // Lens Status Response: 0x00, 0x8D, 0x05, 0x01
    // d1: bit 0 (iris close), bit 2 (iris moving). d2: bit 0 (zoom moving)
    const auto lensResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x8DU, 0x05U, 0x01U);
    assert(FujinonSX800::ProtocolParser::parsePacket(lensResp, status));
    assert(status.irisCloseLimit == true);
    assert(status.irisMoving == true);
    assert(status.zoomMoving == true);
}

void test18ByteSerialResponse()
{
    FujinonSX800::CameraStatus status;
    std::vector<std::uint8_t> serialPkt(18U, 0x00U);
    serialPkt[0] = 0xFFU;
    serialPkt[1] = 0x07U;
    const std::string testSerial = "SX800123";
    for (std::size_t i { 0U }; i < testSerial.size(); ++i) {
        serialPkt[2U + i] = static_cast<std::uint8_t>(testSerial[i]);
    }
    serialPkt[17] = FujinonSX800::PelcoDFrame::calculateChecksum(&serialPkt[1], 16U);

    assert(FujinonSX800::ProtocolParser::parsePacket(serialPkt, status, "QuerySerial"));
    assert(status.serialNumber == "SX800123");
}

void testFwVersionVariants()
{
    FujinonSX800::CameraStatus status;

    // 0x7D variant
    const auto fw7D = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x7DU, 0x03U, 0x0EU);
    assert(FujinonSX800::ProtocolParser::parsePacket(fw7D, status, "QueryFw"));
    assert(status.fwVersionMajor == 3U && status.fwVersionMinor == 14U);

    // 18-byte variant
    std::vector<std::uint8_t> fw18(18U, 0x00U);
    fw18[0] = 0xFFU;
    fw18[1] = 0x07U;
    fw18[2] = 4U;
    fw18[3] = 1U;
    fw18[17] = FujinonSX800::PelcoDFrame::calculateChecksum(&fw18[1], 16U);
    assert(FujinonSX800::ProtocolParser::parsePacket(fw18, status, "QueryFw"));
    assert(status.fwVersionMajor == 4U && status.fwVersionMinor == 1U);
}

int main()
{
    testGeneralAck();
    testExtendedResponses();
    test18ByteSerialResponse();
    testFwVersionVariants();

    std::cout << "[PASS] TestProtocolParser completed successfully." << std::endl;
    return 0;
}

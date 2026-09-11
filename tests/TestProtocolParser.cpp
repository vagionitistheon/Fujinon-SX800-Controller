/// @file TestProtocolParser.cpp
/// @brief Unit tests for Pelco-D response decoding and telemetry population.

#include "FujinonSX800Core/PelcoDFrame.h"
#include "FujinonSX800Core/ProtocolParser.h"
#include "TestHelper.h"

#include <iostream>

void testGeneralAck()
{
    FujinonSX800::CameraStatus status;
    const std::vector<std::uint8_t> ack { 0xFFU, 0x07U, 0x00U, 0x07U };
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(ack, status));
}

void testExtendedResponses()
{
    FujinonSX800::CameraStatus status;

    // Zoom Position Response: 0x00, 0x5D, 0x12, 0x34
    const auto zoomResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x5DU, 0x12U, 0x34U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(zoomResp, status));
    SX800_TEST_ASSERT(status.zoomPosition == 0x1234U);
    SX800_TEST_ASSERT(status.focalLengthMm > 20.0);

    // Focus Position Response: 0x00, 0x81, 0x20, 0x00
    const auto focusResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x81U, 0x20U, 0x00U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(focusResp, status));
    SX800_TEST_ASSERT(status.focusPosition == 0x2000U);

    // FW Version Response: 0x00, 0x8B, 0x02, 0x0C (v2.12)
    const auto fwResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x8BU, 0x02U, 0x0CU);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(fwResp, status));
    SX800_TEST_ASSERT(status.fwVersionMajor == 2U && status.fwVersionMinor == 12U);

    // Lens Status Response: 0x00, 0x8D, 0x05, 0x01
    // d1: bit 0 (iris close), bit 2 (iris moving). d2: bit 0 (zoom moving)
    const auto lensResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x8DU, 0x05U, 0x01U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(lensResp, status));
    SX800_TEST_ASSERT(status.irisCloseLimit == true);
    SX800_TEST_ASSERT(status.irisMoving == true);
    SX800_TEST_ASSERT(status.zoomMoving == true);
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

    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(serialPkt, status, "QuerySerial"));
    SX800_TEST_ASSERT(status.serialNumber == "SX800123");
}

void testFwVersionVariants()
{
    FujinonSX800::CameraStatus status;

    // 0x7D variant
    const auto fw7D = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x7DU, 0x03U, 0x0EU);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(fw7D, status, "QueryFw"));
    SX800_TEST_ASSERT(status.fwVersionMajor == 3U && status.fwVersionMinor == 14U);

    // 18-byte variant
    std::vector<std::uint8_t> fw18(18U, 0x00U);
    fw18[0] = 0xFFU;
    fw18[1] = 0x07U;
    fw18[2] = 4U;
    fw18[3] = 1U;
    fw18[17] = FujinonSX800::PelcoDFrame::calculateChecksum(&fw18[1], 16U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(fw18, status, "QueryFw"));
    SX800_TEST_ASSERT(status.fwVersionMajor == 4U && status.fwVersionMinor == 1U);
}

void testExtendedQueryResponses()
{
    FujinonSX800::CameraStatus status;

    // 0x1F Photo setting query response: d1=0x13 (OIS), d2=0x01 (OIS mode)
    const auto oisResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x1FU, 0x13U, 0x01U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(oisResp, status));
    SX800_TEST_ASSERT(status.opticalStabilization == FujinonSX800::OpticalStabilization::OIS);

    // 0x1F Photo setting query response: d1=0x11 (IR Wavelength), d2=0x03 (850nm)
    const auto irResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x1FU, 0x11U, 0x03U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(irResp, status));
    SX800_TEST_ASSERT(status.irWavelength == FujinonSX800::IrWavelength::Wave850nm);

    // 0x3F Image quality query response: d1=0x2B (Brightness), d2=15
    const auto brightResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x3FU, 0x2BU, 15U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(brightResp, status));
    SX800_TEST_ASSERT(status.brightness == 15U);

    // 0x3F Image quality query response: d1=0x21 (VLC Filter), d2=0x01 (On)
    const auto vlcResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x3FU, 0x21U, 0x01U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(vlcResp, status));
    SX800_TEST_ASSERT(status.vlcFilter == FujinonSX800::VlcFilterMode::On);

    // 0x3F Image quality query response: d1=0x33 (Color Temperature), d2=0x02 (5000K)
    const auto cTemp5000 = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x3FU, 0x33U, 0x02U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(cTemp5000, status));
    SX800_TEST_ASSERT(status.colorTemperatureKelvin == 5000U);

    // 0x3F Image quality query response: d1=0x33 (Color Temperature), d2=0x01 (3000K)
    const auto cTemp3000 = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x3FU, 0x33U, 0x01U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(cTemp3000, status));
    SX800_TEST_ASSERT(status.colorTemperatureKelvin == 3000U);

    // 0x3F Image quality query response: d1=0x33 (Color Temperature), d2=0x03 (9000K)
    const auto cTemp9000 = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x3FU, 0x33U, 0x03U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(cTemp9000, status));
    SX800_TEST_ASSERT(status.colorTemperatureKelvin == 9000U);

    // 0xAF Manual setting query response: d1=0x25 (Zoom speed), d2=8
    const auto spdResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0xAFU, 0x25U, 8U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(spdResp, status));
    SX800_TEST_ASSERT(status.zoomSpeed == 8U);

    // 0xFF Fine settings query response: d1=0xEB (Fine brightness), d2=-5 (0xFB)
    const auto fineBrt = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0xFFU, 0xEBU, 0xFBU);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(fineBrt, status));
    SX800_TEST_ASSERT(status.brightnessFine == -5);
}

void testAllExtendedSpecResponses()
{
    FujinonSX800::CameraStatus status;

    // 0x00, 0x97: Manual Shutter Speed Response: d1=0x20 (1/1000s)
    const auto shutResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x97U, 0x20U, 0x00U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(shutResp, status));
    SX800_TEST_ASSERT(status.shutterSpeed == FujinonSX800::ShutterSpeed::Speed1_1000);

    // 0x00, 0x9B: Manual ISO Response: d1=0x04 (ISO 800)
    const auto isoResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x9BU, 0x04U, 0x00U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(isoResp, status));
    SX800_TEST_ASSERT(status.isoSensitivity == FujinonSX800::IsoSensitivity::Iso800);

    // 0x00, 0x9D: Manual Iris Response: d1=0x07 (F8.0)
    const auto irisResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0x00U, 0x9DU, 0x07U, 0x00U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(irisResp, status));
    SX800_TEST_ASSERT(status.manualIrisFNo == FujinonSX800::ManualIrisFNo::F8_0);

    // 0xF0, 0x5F: Display Settings: d1=0x43, d2=0x01 (Date/Time ON)
    const auto dispResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x5FU, 0x43U, 0x01U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(dispResp, status));
    SX800_TEST_ASSERT(status.dateTimeDisplay == true);

    // 0xF0, 0x8F: Operation Settings: d1=0x67, d2=0x02 (PAL)
    const auto opResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0x8FU, 0x67U, 0x02U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(opResp, status));
    SX800_TEST_ASSERT(status.videoStandard == FujinonSX800::VideoStandard::PAL);

    // 0xF0, 0xB3: SD Movie Count: d1=0x01, d2=0x20 (288 files)
    const auto sdCountResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF0U, 0xB3U, 0x01U, 0x20U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(sdCountResp, status));
    SX800_TEST_ASSERT(status.sdMovieCount == 288U);

    // 0xF1, 0x1F: Day/Night Ex: d1=0x01, d2=0x02 (Night)
    const auto dnExResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF1U, 0x1FU, 0x01U, 0x02U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(dnExResp, status));
    SX800_TEST_ASSERT(status.dayNightMode == FujinonSX800::DayNightMode::Night);

    // 0xF1, 0x2F: Speed Ex: d1=0x25, d2=0x09 (Zoom speed 9)
    const auto spdExResp = FujinonSX800::PelcoDFrame::createFrame(0x07U, 0xF1U, 0x2FU, 0x25U, 0x09U);
    SX800_TEST_ASSERT(FujinonSX800::ProtocolParser::parsePacket(spdExResp, status));
    SX800_TEST_ASSERT(status.zoomSpeed == 9U);
}

int main()
{
    testGeneralAck();
    testExtendedResponses();
    test18ByteSerialResponse();
    testExtendedQueryResponses();
    testAllExtendedSpecResponses();
    testFwVersionVariants();

    std::cout << "[PASS] TestProtocolParser completed successfully." << std::endl;
    return 0;
}

/// @file TestProtocolBuilder.cpp
/// @brief Unit tests verifying wire packet construction across all command categories.

#include "FujinonSX800Core/PelcoDFrame.h"
#include "FujinonSX800Core/ProtocolBuilder.h"
#include "TestHelper.h"

#include <iostream>

void testStandardCommands()
{
    FujinonSX800::ProtocolBuilder builder(0x07U);

    // Zoom Tele: [0xFF, 0x07, 0x00, 0x20, 0x00, 0x00, 0x27]
    const auto tele = builder.buildZoom(FujinonSX800::ZoomDirection::Tele);
    SX800_TEST_ASSERT(tele.size() == 7U);
    SX800_TEST_ASSERT(tele[0] == 0xFFU && tele[1] == 0x07U && tele[2] == 0x00U && tele[3] == 0x20U && tele[6] == 0x27U);

    // Zoom Wide: [0xFF, 0x07, 0x00, 0x40, 0x00, 0x00, 0x47]
    const auto wide = builder.buildZoom(FujinonSX800::ZoomDirection::Wide);
    SX800_TEST_ASSERT(wide[3] == 0x40U && wide[6] == 0x47U);

    // Focus Near: [0xFF, 0x07, 0x01, 0x00, 0x00, 0x00, 0x08]
    const auto near = builder.buildFocus(FujinonSX800::FocusDirection::Near);
    SX800_TEST_ASSERT(near[2] == 0x01U && near[6] == 0x08U);

    // Focus Far: [0xFF, 0x07, 0x00, 0x80, 0x00, 0x00, 0x87]
    const auto far = builder.buildFocus(FujinonSX800::FocusDirection::Far);
    SX800_TEST_ASSERT(far[3] == 0x80U && far[6] == 0x87U);
}

void testCorrectedSpecOpcodes()
{
    FujinonSX800::ProtocolBuilder builder(0x07U);

    // 5.2.9 Set Remote Baud Rate: CMND2 must be 0x67 (spec Section 5.2.9)
    const auto baud = builder.buildSetBaudRate(FujinonSX800::BaudRate::Baud9600);
    SX800_TEST_ASSERT(baud[2] == 0x00U && baud[3] == 0x67U);

    // 5.3.1 Query Focus Position: CMND2 must be 0x81 (spec Section 5.3.1)
    const auto qFocus = builder.buildQueryFocusPosition();
    SX800_TEST_ASSERT(qFocus[2] == 0x00U && qFocus[3] == 0x81U);

    // 5.3.5 Query Serial Number: CMND2 must be 0x89 (spec Section 5.3.5)
    const auto qSerial = builder.buildQuerySerialNumber();
    SX800_TEST_ASSERT(qSerial[2] == 0x00U && qSerial[3] == 0x89U);

    // 5.3.7 Query Lens Status: CMND2 must be 0x8D (spec Section 5.3.7)
    const auto qLens = builder.buildQueryLensStatus();
    SX800_TEST_ASSERT(qLens[2] == 0x00U && qLens[3] == 0x8DU);

    // 5.3.11 Set Manual Shutter Speed: CMND2 must be 0x95 (spec Section 5.3.11)
    const auto shut = builder.buildSetShutterSpeed(FujinonSX800::ShutterSpeed::Speed1_1000);
    SX800_TEST_ASSERT(shut[2] == 0x00U && shut[3] == 0x95U);

    // 5.3.12 Query Manual Shutter Speed: CMND2 must be 0x97 (spec Section 5.3.12)
    const auto qShut = builder.buildQueryShutterSpeed();
    SX800_TEST_ASSERT(qShut[2] == 0x00U && qShut[3] == 0x97U);

    // 5.3.13 Set Manual ISO: CMND2 must be 0x99 (spec Section 5.3.13)
    const auto iso = builder.buildSetIso(FujinonSX800::IsoSensitivity::Iso800);
    SX800_TEST_ASSERT(iso[2] == 0x00U && iso[3] == 0x99U);

    // 5.3.14 Query Manual ISO: CMND2 must be 0x9B (spec Section 5.3.14)
    const auto qIso = builder.buildQueryIso();
    SX800_TEST_ASSERT(qIso[2] == 0x00U && qIso[3] == 0x9BU);

    // 5.3.15 Query Manual Iris: CMND2 must be 0x9D (spec Section 5.3.15)
    const auto qIris = builder.buildQueryManualIris();
    SX800_TEST_ASSERT(qIris[2] == 0x00U && qIris[3] == 0x9DU);
    const auto qIrisPos = builder.buildQueryIrisPosition();
    SX800_TEST_ASSERT(qIrisPos == qIris);

    // 5.3.23 Query Manual Settings: CMND2 must be 0xAD (spec Section 5.3.23)
    const auto qManual = builder.buildQueryManualSettings();
    SX800_TEST_ASSERT(qManual[2] == 0x00U && qManual[3] == 0xADU);

    // 5.4.1 Set AF Area: CMND2 must be 0x03 (spec Section 5.4.1)
    const auto afArea = builder.buildSetAfArea(FujinonSX800::AfArea::Center);
    SX800_TEST_ASSERT(afArea[2] == 0xF0U && afArea[3] == 0x03U);

    // 5.4.2 Set AF Sensitivity: CMND2 must be 0x05 (spec Section 5.4.2)
    const auto afSens = builder.buildSetAfSensitivity(FujinonSX800::AfSensitivity::High);
    SX800_TEST_ASSERT(afSens[2] == 0xF0U && afSens[3] == 0x05U);

    // 5.4.14 Query Photo Settings: CMND2 must be 0x1D (spec Section 5.4.14)
    const auto qPhoto = builder.buildQueryPhotoSettings();
    SX800_TEST_ASSERT(qPhoto[2] == 0xF0U && qPhoto[3] == 0x1DU);

    // 5.5.2 Set WDR: CMND2 must be 0x23 (spec Section 5.5.2)
    const auto wdr = builder.buildSetWdr(FujinonSX800::WdrMode::Low);
    SX800_TEST_ASSERT(wdr[2] == 0xF0U && wdr[3] == 0x23U);

    // 5.5.10 Set Color Temperature: CMND2 must be 0x33 (spec Section 5.5.10)
    const auto cTemp = builder.buildSetColorTemperature(5500U);
    SX800_TEST_ASSERT(cTemp[2] == 0xF0U && cTemp[3] == 0x33U);

    // 5.5.11 Set White Balance: CMND2 must be 0x35 (spec Section 5.5.11)
    const auto wb = builder.buildSetWhiteBalance(FujinonSX800::WhiteBalanceMode::Outdoor);
    SX800_TEST_ASSERT(wb[2] == 0xF0U && wb[3] == 0x35U);

    // 5.5.15 Query Image Quality: CMND2 must be 0x3D (spec Section 5.5.15)
    const auto qImg = builder.buildQueryImageQuality();
    SX800_TEST_ASSERT(qImg[2] == 0xF0U && qImg[3] == 0x3DU);

    // 5.6.2 Set DateTime Display: CMND2 must be 0x43 (spec Section 5.6.2)
    const auto dtDisp = builder.buildSetDateTimeDisplay(true);
    SX800_TEST_ASSERT(dtDisp[2] == 0xF0U && dtDisp[3] == 0x43U);

    // 5.6.4 Set Title Display: CMND2 must be 0x47 (spec Section 5.6.4)
    const auto titleDisp = builder.buildSetTitleDisplay(true);
    SX800_TEST_ASSERT(titleDisp[2] == 0xF0U && titleDisp[3] == 0x47U);

    // 5.6.7 Set ID Display: CMND2 must be 0x4D (spec Section 5.6.7)
    const auto idDisp = builder.buildSetIdDisplay(true);
    SX800_TEST_ASSERT(idDisp[2] == 0xF0U && idDisp[3] == 0x4DU);

    // 5.6.9 Set Reticle Display: CMND2 must be 0x51 (spec Section 5.6.9)
    const auto reticleDisp = builder.buildSetReticleDisplay(true);
    SX800_TEST_ASSERT(reticleDisp[2] == 0xF0U && reticleDisp[3] == 0x51U);

    // 5.6.11 Set Antialiasing: CMND2 must be 0x55 (spec Section 5.6.11)
    const auto aaDisp = builder.buildSetAntialiasing(true);
    SX800_TEST_ASSERT(aaDisp[2] == 0xF0U && aaDisp[3] == 0x55U);

    // 5.6.15 Query Display Settings: CMND2 must be 0x5D (spec Section 5.6.15)
    const auto qDisp = builder.buildQueryDisplaySettings();
    SX800_TEST_ASSERT(qDisp[2] == 0xF0U && qDisp[3] == 0x5DU);

    // 5.7.4 Set Video Standard: CMND2 must be 0x67 (spec Section 5.7.4)
    const auto vidStd = builder.buildSetVideoStandard(FujinonSX800::VideoStandard::NTSC);
    SX800_TEST_ASSERT(vidStd[2] == 0xF0U && vidStd[3] == 0x67U);

    // 5.7.5 Set HD Format: CMND2 must be 0x69 (spec Section 5.7.5)
    const auto hdFmt = builder.buildSetHdFormat(FujinonSX800::HdVideoFormat::HD1080p_60);
    SX800_TEST_ASSERT(hdFmt[2] == 0xF0U && hdFmt[3] == 0x69U);

    // 5.7.10 Set Termination: CMND2 must be 0x73 (spec Section 5.7.10)
    const auto term = builder.buildSetTermination(true);
    SX800_TEST_ASSERT(term[2] == 0xF0U && term[3] == 0x73U);

    // 5.7.23 Query Operation Settings: CMND2 must be 0x8D (spec Section 5.7.23)
    const auto qOp = builder.buildQueryOperationSettings();
    SX800_TEST_ASSERT(qOp[2] == 0xF0U && qOp[3] == 0x8DU);

    // 5.11 Fine Settings: Opcodes 0xEB - 0xFD (spec Section 5.11.6 - 5.11.15)
    const auto fBright = builder.buildSetFineBrightness(5);
    SX800_TEST_ASSERT(fBright[2] == 0xF0U && fBright[3] == 0xEBU);

    const auto fCont = builder.buildSetFineContrast(5);
    SX800_TEST_ASSERT(fCont[2] == 0xF0U && fCont[3] == 0xEDU);

    const auto fSat = builder.buildSetFineSaturation(5);
    SX800_TEST_ASSERT(fSat[2] == 0xF0U && fSat[3] == 0xEFU);

    const auto fSharp = builder.buildSetFineSharpness(5);
    SX800_TEST_ASSERT(fSharp[2] == 0xF0U && fSharp[3] == 0xF1U);

    const auto fWbRed = builder.buildSetWbRedShift(5);
    SX800_TEST_ASSERT(fWbRed[2] == 0xF0U && fWbRed[3] == 0xF5U);

    const auto fWbBlue = builder.buildSetWbBlueShift(5);
    SX800_TEST_ASSERT(fWbBlue[2] == 0xF0U && fWbBlue[3] == 0xF7U);

    const auto qFine = builder.buildQueryFineSettings();
    SX800_TEST_ASSERT(qFine[2] == 0xF0U && qFine[3] == 0xFDU);

    // 5.12 Day/Night Ex: Filters 0x0D/0x0F, Query 0x1D (spec Section 5.12.7 - 5.12.10)
    const auto optDay = builder.buildSetOpticalFilterDay(FujinonSX800::OpticalFilter::Visible);
    SX800_TEST_ASSERT(optDay[2] == 0xF1U && optDay[3] == 0x0DU);

    const auto optNight = builder.buildSetOpticalFilterNight(FujinonSX800::OpticalFilter::IR);
    SX800_TEST_ASSERT(optNight[2] == 0xF1U && optNight[3] == 0x0FU);

    const auto qDnEx = builder.buildQueryDayNightEx();
    SX800_TEST_ASSERT(qDnEx[2] == 0xF1U && qDnEx[3] == 0x1DU);

    // 5.13 Speed Ex: Zoom Speed 0x25, Focus Speed 0x27, Query 0x2D (spec Section 5.13)
    const auto zSpdEx = builder.buildSetZoomSpeedEx(8U);
    SX800_TEST_ASSERT(zSpdEx[2] == 0xF1U && zSpdEx[3] == 0x25U);

    const auto fSpdEx = builder.buildSetFocusSpeedEx(5U);
    SX800_TEST_ASSERT(fSpdEx[2] == 0xF1U && fSpdEx[3] == 0x27U);

    const auto qSpdEx = builder.buildQuerySpeedEx();
    SX800_TEST_ASSERT(qSpdEx[2] == 0xF1U && qSpdEx[3] == 0x2DU);
}

void testRtcClockCommands()
{
    FujinonSX800::ProtocolBuilder builder(0x07U);

    // 1. Set Clock Year (0x06, 0x77): year 2026 -> 0x07EA
    const auto yr = builder.buildSetClockYear(2026U);
    SX800_TEST_ASSERT(yr.size() == 7U);
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::isValidFrame(yr));
    SX800_TEST_ASSERT(yr[1] == 0x07U && yr[2] == 0x06U && yr[3] == 0x77U && yr[4] == 0x07U && yr[5] == 0xEAU);

    // 2. Set Clock Date (0x04, 0x77): September (9), 11
    const auto dt = builder.buildSetClockDate(9U, 11U);
    SX800_TEST_ASSERT(dt.size() == 7U);
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::isValidFrame(dt));
    SX800_TEST_ASSERT(dt[1] == 0x07U && dt[2] == 0x04U && dt[3] == 0x77U && dt[4] == 0x09U && dt[5] == 0x0BU);

    // 3. Set Clock Time (0x02, 0x77): 18:30
    const auto tm = builder.buildSetClockTime(18U, 30U);
    SX800_TEST_ASSERT(tm.size() == 7U);
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::isValidFrame(tm));
    SX800_TEST_ASSERT(tm[1] == 0x07U && tm[2] == 0x02U && tm[3] == 0x77U && tm[4] == 0x12U && tm[5] == 0x1EU);

    // 4. Set Clock Second & Sync (0x00, 0x77): second 45
    const auto sec = builder.buildSetClockSecond(45U);
    SX800_TEST_ASSERT(sec.size() == 7U);
    SX800_TEST_ASSERT(FujinonSX800::PelcoDFrame::isValidFrame(sec));
    SX800_TEST_ASSERT(sec[1] == 0x07U && sec[2] == 0x00U && sec[3] == 0x77U && sec[4] == 0x00U && sec[5] == 0x2DU);

    // 5. Query Clock Commands
    const auto qYr = builder.buildQueryClockYear();
    SX800_TEST_ASSERT(qYr.size() == 7U && FujinonSX800::PelcoDFrame::isValidFrame(qYr));
    SX800_TEST_ASSERT(qYr[2] == 0x07U && qYr[3] == 0x77U && qYr[4] == 0x00U && qYr[5] == 0x00U);

    const auto qDt = builder.buildQueryClockDate();
    SX800_TEST_ASSERT(qDt.size() == 7U && FujinonSX800::PelcoDFrame::isValidFrame(qDt));
    SX800_TEST_ASSERT(qDt[2] == 0x05U && qDt[3] == 0x77U && qDt[4] == 0x00U && qDt[5] == 0x00U);

    const auto qTm = builder.buildQueryClockTime();
    SX800_TEST_ASSERT(qTm.size() == 7U && FujinonSX800::PelcoDFrame::isValidFrame(qTm));
    SX800_TEST_ASSERT(qTm[2] == 0x03U && qTm[3] == 0x77U && qTm[4] == 0x00U && qTm[5] == 0x00U);

    const auto qSec = builder.buildQueryClockSecond();
    SX800_TEST_ASSERT(qSec.size() == 7U && FujinonSX800::PelcoDFrame::isValidFrame(qSec));
    SX800_TEST_ASSERT(qSec[2] == 0x01U && qSec[3] == 0x77U && qSec[4] == 0x00U && qSec[5] == 0x00U);

    // 6. Multi-frame buildSetRtcTime (full 28 bytes)
    const auto multi = builder.buildSetRtcTime(static_cast<std::uint16_t>(2026U), 9U, 11U, 18U, 30U, 45U);
    SX800_TEST_ASSERT(multi.size() == 28U);
    const auto frames = FujinonSX800::PelcoDFrame::splitStream(multi);
    SX800_TEST_ASSERT(frames.size() == 4U);
    SX800_TEST_ASSERT(frames[0] == yr);
    SX800_TEST_ASSERT(frames[1] == dt);
    SX800_TEST_ASSERT(frames[2] == tm);
    SX800_TEST_ASSERT(frames[3] == sec);

    // 7. Multi-frame buildSetRtcTime with 2-digit year
    const auto multi2 = builder.buildSetRtcTime(static_cast<std::uint8_t>(26U), 9U, 11U, 18U, 30U, 45U);
    SX800_TEST_ASSERT(multi2 == multi);
}

void testManualIrisCommands()
{
    using FujinonSX800::ManualIrisFNo;
    using FujinonSX800::PelcoDFrame;
    using FujinonSX800::positionToManualIrisFNo;

    FujinonSX800::ProtocolBuilder builder(0x07U);

    // 1. Set Manual Iris (spec Section 5.3.9): CMND2=0x91, DATA1=0x00, DATA2=fNo
    const auto f5_6 = builder.buildSetManualIris(ManualIrisFNo::F5_6);
    SX800_TEST_ASSERT(f5_6.size() == 7U);
    SX800_TEST_ASSERT(PelcoDFrame::isValidFrame(f5_6));
    SX800_TEST_ASSERT(f5_6[1] == 0x07U && f5_6[2] == 0x00U && f5_6[3] == 0x91U);
    SX800_TEST_ASSERT(f5_6[4] == 0x00U && f5_6[5] == 0x04U);

    // 2. Query Manual Iris (spec Section 5.3.15): CMND2=0x9D, DATA1=0x00, DATA2=0x00
    const auto qIris = builder.buildQueryManualIris();
    SX800_TEST_ASSERT(qIris.size() == 7U);
    SX800_TEST_ASSERT(PelcoDFrame::isValidFrame(qIris));
    SX800_TEST_ASSERT(qIris[1] == 0x07U && qIris[2] == 0x00U && qIris[3] == 0x9DU);
    SX800_TEST_ASSERT(qIris[4] == 0x00U && qIris[5] == 0x00U);

    // 3. buildSetIrisPosition: ensures no truncation of 16-bit positions > 0xFF
    const auto p512 = builder.buildSetIrisPosition(512U);
    SX800_TEST_ASSERT(p512.size() == 7U);
    SX800_TEST_ASSERT(PelcoDFrame::isValidFrame(p512));
    SX800_TEST_ASSERT(p512[2] == 0x00U && p512[3] == 0x91U);
    SX800_TEST_ASSERT(p512[4] == 0x00U && p512[5] != 0x00U); // must NOT be corrupted to 0x00
    SX800_TEST_ASSERT(p512[5] >= 0x01U && p512[5] <= 0x12U);

    const auto p2000 = builder.buildSetIrisPosition(0x2000U);
    SX800_TEST_ASSERT(p2000.size() == 7U);
    SX800_TEST_ASSERT(PelcoDFrame::isValidFrame(p2000));
    SX800_TEST_ASSERT(p2000[2] == 0x00U && p2000[3] == 0x91U);
    SX800_TEST_ASSERT(p2000[4] == 0x00U && p2000[5] != 0x00U); // must NOT be corrupted to 0x00
    SX800_TEST_ASSERT(p2000[5] >= 0x01U && p2000[5] <= 0x12U);

    const auto pClose = builder.buildSetIrisPosition(1023U);
    SX800_TEST_ASSERT(pClose[5] == static_cast<std::uint8_t>(ManualIrisFNo::Close));

    const auto pZero = builder.buildSetIrisPosition(0U);
    SX800_TEST_ASSERT(pZero[5] == static_cast<std::uint8_t>(ManualIrisFNo::F4_0));

    // 4. positionToManualIrisFNo helper assertions
    SX800_TEST_ASSERT(positionToManualIrisFNo(0U) == ManualIrisFNo::F4_0);
    SX800_TEST_ASSERT(positionToManualIrisFNo(1U) == ManualIrisFNo::F4_0);
    SX800_TEST_ASSERT(positionToManualIrisFNo(18U) == ManualIrisFNo::Close);
    SX800_TEST_ASSERT(positionToManualIrisFNo(512U) == ManualIrisFNo::F11);
    SX800_TEST_ASSERT(positionToManualIrisFNo(0x2000U) == ManualIrisFNo::F11);
    SX800_TEST_ASSERT(positionToManualIrisFNo(0x4000U) == ManualIrisFNo::Close);
    SX800_TEST_ASSERT(positionToManualIrisFNo(65535U) == ManualIrisFNo::Close);
}

int main()
{
    testStandardCommands();
    testCorrectedSpecOpcodes();
    testRtcClockCommands();
    testManualIrisCommands();

    std::cout << "[PASS] TestProtocolBuilder completed successfully." << std::endl;
    return 0;
}

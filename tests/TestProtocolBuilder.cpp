/// @file TestProtocolBuilder.cpp
/// @brief Unit tests verifying wire packet construction across all command categories.

#include "FujinonSX800Core/ProtocolBuilder.h"

#include <cassert>
#include <iostream>

void testStandardCommands()
{
    FujinonSX800::ProtocolBuilder builder(0x07U);

    // Zoom Tele: [0xFF, 0x07, 0x00, 0x20, 0x00, 0x00, 0x27]
    const auto tele = builder.buildZoom(FujinonSX800::ZoomDirection::Tele);
    assert(tele.size() == 7U);
    assert(tele[0] == 0xFFU && tele[1] == 0x07U && tele[2] == 0x00U && tele[3] == 0x20U && tele[6] == 0x27U);

    // Zoom Wide: [0xFF, 0x07, 0x00, 0x40, 0x00, 0x00, 0x47]
    const auto wide = builder.buildZoom(FujinonSX800::ZoomDirection::Wide);
    assert(wide[3] == 0x40U && wide[6] == 0x47U);

    // Focus Near: [0xFF, 0x07, 0x01, 0x00, 0x00, 0x00, 0x08]
    const auto near = builder.buildFocus(FujinonSX800::FocusDirection::Near);
    assert(near[2] == 0x01U && near[6] == 0x08U);

    // Focus Far: [0xFF, 0x07, 0x00, 0x80, 0x00, 0x00, 0x87]
    const auto far = builder.buildFocus(FujinonSX800::FocusDirection::Far);
    assert(far[3] == 0x80U && far[6] == 0x87U);
}

void testCorrectedSpecOpcodes()
{
    FujinonSX800::ProtocolBuilder builder(0x07U);

    // 5.2.9 Set Remote Baud Rate: CMND2 must be 0x67 (spec Section 5.2.9)
    const auto baud = builder.buildSetBaudRate(FujinonSX800::BaudRate::Baud9600);
    assert(baud[2] == 0x00U && baud[3] == 0x67U);

    // 5.3.1 Query Focus Position: CMND2 must be 0x81 (spec Section 5.3.1)
    const auto qFocus = builder.buildQueryFocusPosition();
    assert(qFocus[2] == 0x00U && qFocus[3] == 0x81U);

    // 5.3.5 Query Serial Number: CMND2 must be 0x89 (spec Section 5.3.5)
    const auto qSerial = builder.buildQuerySerialNumber();
    assert(qSerial[2] == 0x00U && qSerial[3] == 0x89U);

    // 5.3.7 Query Lens Status: CMND2 must be 0x8D (spec Section 5.3.7)
    const auto qLens = builder.buildQueryLensStatus();
    assert(qLens[2] == 0x00U && qLens[3] == 0x8DU);

    // 5.3.11 Set Manual Shutter Speed: CMND2 must be 0x95 (spec Section 5.3.11)
    const auto shut = builder.buildSetShutterSpeed(FujinonSX800::ShutterSpeed::Speed1_1000);
    assert(shut[2] == 0x00U && shut[3] == 0x95U);

    // 5.3.12 Query Manual Shutter Speed: CMND2 must be 0x97 (spec Section 5.3.12)
    const auto qShut = builder.buildQueryShutterSpeed();
    assert(qShut[2] == 0x00U && qShut[3] == 0x97U);

    // 5.3.13 Set Manual ISO: CMND2 must be 0x99 (spec Section 5.3.13)
    const auto iso = builder.buildSetIso(FujinonSX800::IsoSensitivity::Iso800);
    assert(iso[2] == 0x00U && iso[3] == 0x99U);

    // 5.3.14 Query Manual ISO: CMND2 must be 0x9B (spec Section 5.3.14)
    const auto qIso = builder.buildQueryIso();
    assert(qIso[2] == 0x00U && qIso[3] == 0x9BU);

    // 5.3.15 Query Manual Iris: CMND2 must be 0x9D (spec Section 5.3.15)
    const auto qIris = builder.buildQueryIrisPosition();
    assert(qIris[2] == 0x00U && qIris[3] == 0x9DU);

    // 5.3.23 Query Manual Settings: CMND2 must be 0xAD (spec Section 5.3.23)
    const auto qManual = builder.buildQueryManualSettings();
    assert(qManual[2] == 0x00U && qManual[3] == 0xADU);

    // 5.4.1 Set AF Area: CMND2 must be 0x03 (spec Section 5.4.1)
    const auto afArea = builder.buildSetAfArea(FujinonSX800::AfArea::Center);
    assert(afArea[2] == 0xF0U && afArea[3] == 0x03U);

    // 5.4.2 Set AF Sensitivity: CMND2 must be 0x05 (spec Section 5.4.2)
    const auto afSens = builder.buildSetAfSensitivity(FujinonSX800::AfSensitivity::High);
    assert(afSens[2] == 0xF0U && afSens[3] == 0x05U);

    // 5.4.14 Query Photo Settings: CMND2 must be 0x1D (spec Section 5.4.14)
    const auto qPhoto = builder.buildQueryPhotoSettings();
    assert(qPhoto[2] == 0xF0U && qPhoto[3] == 0x1DU);

    // 5.5.2 Set WDR: CMND2 must be 0x23 (spec Section 5.5.2)
    const auto wdr = builder.buildSetWdr(FujinonSX800::WdrMode::Low);
    assert(wdr[2] == 0xF0U && wdr[3] == 0x23U);

    // 5.5.10 Set Color Temperature: CMND2 must be 0x33 (spec Section 5.5.10)
    const auto cTemp = builder.buildSetColorTemperature(5500U);
    assert(cTemp[2] == 0xF0U && cTemp[3] == 0x33U);

    // 5.5.11 Set White Balance: CMND2 must be 0x35 (spec Section 5.5.11)
    const auto wb = builder.buildSetWhiteBalance(FujinonSX800::WhiteBalanceMode::Outdoor);
    assert(wb[2] == 0xF0U && wb[3] == 0x35U);

    // 5.5.15 Query Image Quality: CMND2 must be 0x3D (spec Section 5.5.15)
    const auto qImg = builder.buildQueryImageQuality();
    assert(qImg[2] == 0xF0U && qImg[3] == 0x3DU);

    // 5.6.2 Set DateTime Display: CMND2 must be 0x43 (spec Section 5.6.2)
    const auto dtDisp = builder.buildSetDateTimeDisplay(true);
    assert(dtDisp[2] == 0xF0U && dtDisp[3] == 0x43U);

    // 5.6.4 Set Title Display: CMND2 must be 0x47 (spec Section 5.6.4)
    const auto titleDisp = builder.buildSetTitleDisplay(true);
    assert(titleDisp[2] == 0xF0U && titleDisp[3] == 0x47U);

    // 5.6.7 Set ID Display: CMND2 must be 0x4D (spec Section 5.6.7)
    const auto idDisp = builder.buildSetIdDisplay(true);
    assert(idDisp[2] == 0xF0U && idDisp[3] == 0x4DU);

    // 5.6.9 Set Reticle Display: CMND2 must be 0x51 (spec Section 5.6.9)
    const auto reticleDisp = builder.buildSetReticleDisplay(true);
    assert(reticleDisp[2] == 0xF0U && reticleDisp[3] == 0x51U);

    // 5.6.11 Set Antialiasing: CMND2 must be 0x55 (spec Section 5.6.11)
    const auto aaDisp = builder.buildSetAntialiasing(true);
    assert(aaDisp[2] == 0xF0U && aaDisp[3] == 0x55U);

    // 5.6.15 Query Display Settings: CMND2 must be 0x5D (spec Section 5.6.15)
    const auto qDisp = builder.buildQueryDisplaySettings();
    assert(qDisp[2] == 0xF0U && qDisp[3] == 0x5DU);

    // 5.7.4 Set Video Standard: CMND2 must be 0x67 (spec Section 5.7.4)
    const auto vidStd = builder.buildSetVideoStandard(FujinonSX800::VideoStandard::NTSC);
    assert(vidStd[2] == 0xF0U && vidStd[3] == 0x67U);

    // 5.7.5 Set HD Format: CMND2 must be 0x69 (spec Section 5.7.5)
    const auto hdFmt = builder.buildSetHdFormat(FujinonSX800::HdVideoFormat::HD1080p_60);
    assert(hdFmt[2] == 0xF0U && hdFmt[3] == 0x69U);

    // 5.7.10 Set Termination: CMND2 must be 0x73 (spec Section 5.7.10)
    const auto term = builder.buildSetTermination(true);
    assert(term[2] == 0xF0U && term[3] == 0x73U);

    // 5.7.23 Query Operation Settings: CMND2 must be 0x8D (spec Section 5.7.23)
    const auto qOp = builder.buildQueryOperationSettings();
    assert(qOp[2] == 0xF0U && qOp[3] == 0x8DU);

    // 5.11 Fine Settings: Opcodes 0xEB - 0xFD (spec Section 5.11.6 - 5.11.15)
    const auto fBright = builder.buildSetFineBrightness(5);
    assert(fBright[2] == 0xF0U && fBright[3] == 0xEBU);

    const auto fCont = builder.buildSetFineContrast(5);
    assert(fCont[2] == 0xF0U && fCont[3] == 0xEDU);

    const auto fSat = builder.buildSetFineSaturation(5);
    assert(fSat[2] == 0xF0U && fSat[3] == 0xEFU);

    const auto fSharp = builder.buildSetFineSharpness(5);
    assert(fSharp[2] == 0xF0U && fSharp[3] == 0xF1U);

    const auto fWbRed = builder.buildSetWbRedShift(5);
    assert(fWbRed[2] == 0xF0U && fWbRed[3] == 0xF5U);

    const auto fWbBlue = builder.buildSetWbBlueShift(5);
    assert(fWbBlue[2] == 0xF0U && fWbBlue[3] == 0xF7U);

    const auto qFine = builder.buildQueryFineSettings();
    assert(qFine[2] == 0xF0U && qFine[3] == 0xFDU);

    // 5.12 Day/Night Ex: Filters 0x0D/0x0F, Query 0x1D (spec Section 5.12.7 - 5.12.10)
    const auto optDay = builder.buildSetOpticalFilterDay(FujinonSX800::OpticalFilter::Visible);
    assert(optDay[2] == 0xF1U && optDay[3] == 0x0DU);

    const auto optNight = builder.buildSetOpticalFilterNight(FujinonSX800::OpticalFilter::IR);
    assert(optNight[2] == 0xF1U && optNight[3] == 0x0FU);

    const auto qDnEx = builder.buildQueryDayNightEx();
    assert(qDnEx[2] == 0xF1U && qDnEx[3] == 0x1DU);

    // 5.13 Speed Ex: Zoom Speed 0x25, Focus Speed 0x27, Query 0x2D (spec Section 5.13)
    const auto zSpdEx = builder.buildSetZoomSpeedEx(8U);
    assert(zSpdEx[2] == 0xF1U && zSpdEx[3] == 0x25U);

    const auto fSpdEx = builder.buildSetFocusSpeedEx(5U);
    assert(fSpdEx[2] == 0xF1U && fSpdEx[3] == 0x27U);

    const auto qSpdEx = builder.buildQuerySpeedEx();
    assert(qSpdEx[2] == 0xF1U && qSpdEx[3] == 0x2DU);
}

int main()
{
    testStandardCommands();
    testCorrectedSpecOpcodes();

    std::cout << "[PASS] TestProtocolBuilder completed successfully." << std::endl;
    return 0;
}

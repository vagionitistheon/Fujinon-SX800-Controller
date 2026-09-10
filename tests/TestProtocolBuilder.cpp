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

void testExtendedAndOriginalCommands()
{
    FujinonSX800::ProtocolBuilder builder(0x07U);

    // Set Zoom Position (pos=0x1234): [0xFF, 0x07, 0x00, 0x4F, 0x12, 0x34, CKSM]
    const auto zoomPos = builder.buildSetZoomPosition(0x1234U);
    assert(zoomPos[2] == 0x00U && zoomPos[3] == 0x4FU && zoomPos[4] == 0x12U && zoomPos[5] == 0x34U);

    // Set Focus Position (pos=0x5678): [0xFF, 0x07, 0x00, 0x8F, 0x56, 0x78, CKSM]
    const auto focusPos = builder.buildSetFocusPosition(0x5678U);
    assert(focusPos[2] == 0x00U && focusPos[3] == 0x8FU && focusPos[4] == 0x56U && focusPos[5] == 0x78U);

    // One-push AF: [0xFF, 0x07, 0xF0, 0x07, 0x00, 0x00, CKSM]
    const auto pushAf = builder.buildOnePushAf();
    assert(pushAf[2] == 0xF0U && pushAf[3] == 0x07U);

    // Set VLC filter On: [0xFF, 0x07, 0xF0, 0x21, 0x00, 0x01, CKSM]
    const auto vlc = builder.buildSetVlcFilter(FujinonSX800::VlcFilterMode::On);
    assert(vlc[2] == 0xF0U && vlc[3] == 0x21U && vlc[5] == 0x01U);

    // Set Defog High: [0xFF, 0x07, 0xF0, 0x29, 0x00, 0x03, CKSM]
    const auto defog = builder.buildSetDefog(FujinonSX800::DefogMode::High);
    assert(defog[2] == 0xF0U && defog[3] == 0x29U && defog[5] == 0x03U);

    // Set De-Heat Haze Mid: [0xFF, 0x07, 0xF0, 0x27, 0x00, 0x02, CKSM]
    const auto haze = builder.buildSetDeHeatHaze(FujinonSX800::DeHeatHazeMode::Mid);
    assert(haze[2] == 0xF0U && haze[3] == 0x27U && haze[5] == 0x02U);

    // Speed Ex: [0xFF, 0x07, 0xF1, 0x21, 0x00, 0x08, CKSM]
    const auto spdEx = builder.buildSetZoomSpeedEx(8U);
    assert(spdEx[2] == 0xF1U && spdEx[3] == 0x21U && spdEx[5] == 0x08U);

    // Reboot: [0xFF, 0x07, 0xF0, 0x83, 0x00, 0x01, CKSM]
    const auto reboot = builder.buildReboot();
    assert(reboot[2] == 0xF0U && reboot[3] == 0x83U && reboot[4] == 0x00U && reboot[5] == 0x01U);

    // Factory Reset: [0xFF, 0x07, 0xF0, 0x81, 0x00, 0x00, CKSM]
    const auto reset = builder.buildFactoryReset();
    assert(reset[2] == 0xF0U && reset[3] == 0x81U && reset[4] == 0x00U && reset[5] == 0x00U);

    // Set Manual Day/Night: [0xFF, 0x07, 0xF0, 0x0F, 0x00, mode, CKSM]
    const auto dn = builder.buildSetDayNight(FujinonSX800::DayNightMode::Night);
    assert(dn[2] == 0xF0U && dn[3] == 0x0FU && dn[5] == static_cast<std::uint8_t>(FujinonSX800::DayNightMode::Night));

    // Set IR Wavelength 850nm: [0xFF, 0x07, 0xF0, 0x11, 0x00, 0x03, CKSM]
    const auto ir = builder.buildSetIrWavelength(FujinonSX800::IrWavelength::Wave850nm);
    assert(ir[2] == 0xF0U && ir[3] == 0x11U && ir[5] == 0x03U);

    // Set OIS Mode: [0xFF, 0x07, 0xF0, 0x13, 0x00, mode, CKSM]
    const auto ois = builder.buildSetOpticalStabilization(FujinonSX800::OpticalStabilization::OIS);
    assert(ois[2] == 0xF0U && ois[3] == 0x13U);
}

int main()
{
    testStandardCommands();
    testExtendedAndOriginalCommands();

    std::cout << "[PASS] TestProtocolBuilder completed successfully." << std::endl;
    return 0;
}

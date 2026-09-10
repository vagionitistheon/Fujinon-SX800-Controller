#include "ProtocolBuilder.h"

namespace FujinonSX800 {

ProtocolBuilder::ProtocolBuilder(std::uint8_t cameraAddress) noexcept
    : m_address { cameraAddress }
{
}

void ProtocolBuilder::setAddress(std::uint8_t address) noexcept
{
    m_address = address;
}

std::uint8_t ProtocolBuilder::getAddress() const noexcept
{
    return m_address;
}

// 5.1 Standard Commands
std::vector<std::uint8_t> ProtocolBuilder::buildZoom(ZoomDirection dir) const
{
    std::uint8_t cmd2 { 0x00U };
    if (dir == ZoomDirection::Tele) {
        cmd2 = 0x20U;
    } else if (dir == ZoomDirection::Wide) {
        cmd2 = 0x40U;
    }
    return PelcoDFrame::createFrame(m_address, 0x00U, cmd2, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildFocus(FocusDirection dir) const
{
    std::uint8_t cmd1 { 0x00U };
    std::uint8_t cmd2 { 0x00U };
    if (dir == FocusDirection::Near) {
        cmd1 = 0x01U;
    } else if (dir == FocusDirection::Far) {
        cmd2 = 0x80U;
    }
    return PelcoDFrame::createFrame(m_address, cmd1, cmd2, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildIris(IrisDirection dir) const
{
    std::uint8_t cmd1 { 0x00U };
    std::uint8_t cmd2 { 0x00U };
    if (dir == IrisDirection::Open) {
        cmd1 = 0x02U;
    } else if (dir == IrisDirection::Close) {
        cmd1 = 0x04U;
    }
    return PelcoDFrame::createFrame(m_address, cmd1, cmd2, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildStop() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x00U, 0x00U, 0x00U);
}

// 5.2 Extended Commands
std::vector<std::uint8_t> ProtocolBuilder::buildSetZoomSpeed(std::uint8_t speed) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x25U, 0x00U, speed);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFocusSpeed(std::uint8_t speed) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x27U, 0x00U, speed);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAutoFocus(AutoFocusMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x2BU, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAutoIris(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x02U : 0x01U);
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x2DU, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAgc(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x02U : 0x01U);
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x2FU, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetBlc(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x02U : 0x01U);
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x31U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetZoomPosition(std::uint16_t pulse) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((pulse >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(pulse & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x4FU, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryZoomPosition() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x55U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetBaudRate(BaudRate rate) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x5BU, 0x00U, static_cast<std::uint8_t>(rate));
}

// 5.3 FF Extended Commands
std::vector<std::uint8_t> ProtocolBuilder::buildSetFocusPosition(std::uint16_t pulse) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((pulse >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(pulse & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x8FU, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIrisPosition(std::uint16_t pos) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((pos >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(pos & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x91U, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetShutterSpeed(ShutterSpeed speed) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x93U, 0x00U, static_cast<std::uint8_t>(speed));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIso(IsoSensitivity iso) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x95U, 0x00U, static_cast<std::uint8_t>(iso));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryFocusPosition() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x73U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryIrisPosition() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x75U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryShutterSpeed() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x77U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryIso() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x79U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQuerySerialNumber() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x7BU, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryFwVersion() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x8BU, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryLensStatus() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x7FU, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryManualSettings() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x83U, 0x00U, 0x00U);
}

// 5.4 Original Command 1 (Photo Settings)
std::vector<std::uint8_t> ProtocolBuilder::buildOnePushAf() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x07U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAfArea(AfArea area) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x09U, 0x00U, static_cast<std::uint8_t>(area));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAfSensitivity(AfSensitivity sens) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x0BU, 0x00U, static_cast<std::uint8_t>(sens));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDayNight(DayNightMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x0FU, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIrWavelength(IrWavelength wl) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x11U, 0x00U, static_cast<std::uint8_t>(wl));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetOpticalStabilization(OpticalStabilization mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x13U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryPhotoSettings() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x1FU, 0x00U, 0x00U);
}

// 5.5 Original Command 2 (Image Quality)
std::vector<std::uint8_t> ProtocolBuilder::buildSetVlcFilter(VlcFilterMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x21U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWdr(WdrMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x25U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDeHeatHaze(DeHeatHazeMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x27U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDefog(DefogMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x29U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetBrightness(std::uint8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x2BU, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetContrast(std::uint8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x2DU, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetSaturation(std::uint8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x2FU, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetSharpness(std::uint8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x31U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWhiteBalance(WhiteBalanceMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x33U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetColorTemperature(std::uint16_t kelvin) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((kelvin >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(kelvin & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x35U, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDigitalZoom(DigitalZoomMode mode, std::uint8_t mag) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x37U, static_cast<std::uint8_t>(mode), mag);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetNoiseReduction(NoiseReductionLevel level) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x39U, 0x00U, static_cast<std::uint8_t>(level));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryImageQuality() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x3FU, 0x00U, 0x00U);
}

// 5.6 Original Command 3 (Display Settings / OSD)
std::vector<std::uint8_t> ProtocolBuilder::buildSetDateTimeDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x41U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetTitleDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x43U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIdDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x45U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetReticleDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x47U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAntialiasing(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x49U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryDisplaySettings() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x5FU, 0x00U, 0x00U);
}

// 5.7 Original Command 4 (Operation Settings / System)
std::vector<std::uint8_t> ProtocolBuilder::buildSetVideoStandard(VideoStandard standard) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x61U, 0x00U, static_cast<std::uint8_t>(standard));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetHdFormat(HdVideoFormat format) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x63U, 0x00U, static_cast<std::uint8_t>(format));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetTermination(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x67U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSavePreset(std::uint8_t presetId) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x69U, 0x00U, presetId);
}

std::vector<std::uint8_t> ProtocolBuilder::buildRecallPreset(std::uint8_t presetId) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x6BU, 0x00U, presetId);
}

std::vector<std::uint8_t> ProtocolBuilder::buildReboot() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x83U, 0x00U, 0x01U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildFactoryReset() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x81U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryOperationSettings() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x7FU, 0x00U, 0x00U);
}

// 5.8 Original Command 5 (Key Settings & SD Playback)
std::vector<std::uint8_t> ProtocolBuilder::buildMenuKey(MenuKey key) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x8BU, 0x00U, static_cast<std::uint8_t>(key));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSdPlaybackControl(std::uint8_t action) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x85U, 0x00U, action);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryKeySettings() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x9FU, 0x00U, 0x00U);
}

// 5.9 Original Command 6 (SD Movie Catalog)
std::vector<std::uint8_t> ProtocolBuilder::buildQuerySdMovieCount() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xA1U, 0x00U, 0x00U);
}

// 5.10 Original Command 7 (Log Data)
std::vector<std::uint8_t> ProtocolBuilder::buildQueryLogData(std::uint8_t type) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xC1U, 0x00U, type);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryTemperature() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xC1U, 0x00U, 0x05U);
}

// 5.11 Original Command 8 (Fine Settings)
std::vector<std::uint8_t> ProtocolBuilder::buildSetFineBrightness(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xE1U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFineContrast(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xE3U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFineSaturation(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xE5U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFineSharpness(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xE7U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWbRedShift(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xE9U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWbBlueShift(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xEBU, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryFineSettings() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xEFU, 0x00U, 0x00U);
}

// 5.12 Original Command 9 (Day/Night Extended Settings)
std::vector<std::uint8_t> ProtocolBuilder::buildSetDayNightThreshold(std::uint8_t d2n, std::uint8_t n2d) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x01U, d2n, n2d);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetOpticalFilterDay(OpticalFilter filter) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x07U, 0x00U, static_cast<std::uint8_t>(filter));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetOpticalFilterNight(OpticalFilter filter) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x09U, 0x00U, static_cast<std::uint8_t>(filter));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryDayNightEx() const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x1FU, 0x00U, 0x00U);
}

// 5.13 Original Command 10 (Zoom/Focus Speed Extended)
std::vector<std::uint8_t> ProtocolBuilder::buildSetZoomSpeedEx(std::uint8_t speed) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x21U, 0x00U, speed);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFocusSpeedEx(std::uint8_t speed) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x23U, 0x00U, speed);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQuerySpeedEx() const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x3FU, 0x00U, 0x00U);
}

} // namespace FujinonSX800

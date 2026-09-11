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

// -----------------------------------------------------------------------------
// 5.1 Standard Commands
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// 5.2 Extended Commands
// -----------------------------------------------------------------------------
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
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x67U, 0x00U, static_cast<std::uint8_t>(rate));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetRtcTime(std::uint8_t year, std::uint8_t month, std::uint8_t day,
    std::uint8_t hour, std::uint8_t minute, std::uint8_t second) const
{
    (void)year;
    (void)month;
    (void)day;
    (void)hour;
    (void)minute;
    (void)second;
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x77U, 0x00U, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.3 FF Extended Commands
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildQueryFocusPosition() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x81U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryZoomPositionFf() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x83U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQuerySerialNumber() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x89U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryFwVersion() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x8BU, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryLensStatus() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x8DU, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFocusPosition(std::uint16_t pulse) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((pulse >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(pulse & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x8FU, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetManualIris(ManualIrisFNo fNo) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x91U, 0x00U, static_cast<std::uint8_t>(fNo));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIrisPosition(std::uint16_t pos) const
{
    const auto fNo = static_cast<ManualIrisFNo>(pos & 0xFFU);
    return buildSetManualIris(fNo);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetShutterLimit(ShutterLimitMode limit) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x93U, 0x00U, static_cast<std::uint8_t>(limit));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetShutterSpeed(ShutterSpeed speed) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x95U, 0x00U, static_cast<std::uint8_t>(speed));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryShutterSpeed() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x97U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIso(IsoSensitivity iso) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x99U, 0x00U, static_cast<std::uint8_t>(iso));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryIso() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x9BU, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryIrisPosition() const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0x9DU, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryManualSettings(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0x00U, 0xADU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.4 Original Command 1 (Photo Settings)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetAfArea(AfArea area) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x03U, 0x00U, static_cast<std::uint8_t>(area));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAfSensitivity(AfSensitivity sens) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x05U, 0x00U, static_cast<std::uint8_t>(sens));
}

std::vector<std::uint8_t> ProtocolBuilder::buildOnePushAf() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x07U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAutoDayNight(DayNightMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x0DU, 0x00U, static_cast<std::uint8_t>(mode));
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

std::vector<std::uint8_t> ProtocolBuilder::buildSetPhotoPreset(std::uint8_t preset) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x17U, 0x00U, preset);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDayNightTrigger(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x19U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryPhotoSettings(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x1DU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.5 Original Command 2 (Image Quality)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetVlcFilter(VlcFilterMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x21U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWdr(WdrMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x23U, 0x00U, static_cast<std::uint8_t>(mode));
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

std::vector<std::uint8_t> ProtocolBuilder::buildSetColorTemperature(std::uint16_t kelvin) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((kelvin >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(kelvin & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x33U, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWhiteBalance(WhiteBalanceMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x35U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDigitalZoom(DigitalZoomMode mode, std::uint8_t mag) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x37U, static_cast<std::uint8_t>(mode), mag);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetNoiseReduction(NoiseReductionLevel level) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x39U, 0x00U, static_cast<std::uint8_t>(level));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryImageQuality(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x3DU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.6 Original Command 3 (Display Settings / OSD)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetDateTimeDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x43U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDateTimePosition(OsdPosition pos) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x45U, 0x00U, static_cast<std::uint8_t>(pos));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetTitleDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x47U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetTitlePosition(OsdPosition pos) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x4BU, 0x00U, static_cast<std::uint8_t>(pos));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIdDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x4DU, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetIdPosition(OsdPosition pos) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x4FU, 0x00U, static_cast<std::uint8_t>(pos));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetReticleDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x51U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetAntialiasing(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x55U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryDisplaySettings(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x5DU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.7 Original Command 4 (Operation Settings / System)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetTimeDisplayFormat(TimeDisplayFormat fmt) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x61U, 0x00U, static_cast<std::uint8_t>(fmt));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDateDisplayFormat(DateDisplayFormat fmt) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x63U, 0x00U, static_cast<std::uint8_t>(fmt));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetVideoStandard(VideoStandard standard) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x67U, 0x00U, static_cast<std::uint8_t>(standard));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetHdFormat(HdVideoFormat format) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x69U, 0x00U, static_cast<std::uint8_t>(format));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetVideoDisplayMode(VideoDisplayMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x6BU, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetRs485Id(std::uint8_t id) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x6FU, 0x00U, id);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetTermination(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x73U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetSdRecordingMode(SdRecordMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x77U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetSdCapacityDisplay(bool enable) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(enable ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x79U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildFormatSdCard() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x7BU, 0x00U, 0x01U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildDownloadFirmware() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x7DU, 0x00U, 0x01U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildRecordLogSd() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x7FU, 0x00U, 0x01U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildPreset(PresetAction action, std::uint8_t presetId) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x81U, static_cast<std::uint8_t>(action), presetId);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSavePreset(std::uint8_t presetId) const
{
    return buildPreset(PresetAction::Save, presetId);
}

std::vector<std::uint8_t> ProtocolBuilder::buildRecallPreset(std::uint8_t presetId) const
{
    return buildPreset(PresetAction::Recall, presetId);
}

std::vector<std::uint8_t> ProtocolBuilder::buildReboot() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x83U, 0x00U, 0x01U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildFactoryReset() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x81U, 0x00U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetLanguage(Language lang) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x85U, 0x00U, static_cast<std::uint8_t>(lang));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryOperationSettings(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x8DU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.8 Original Command 5 (Key Settings & SD Playback)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetEncodeMode(std::uint8_t mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x91U, 0x00U, mode);
}

std::vector<std::uint8_t> ProtocolBuilder::buildRecordLiveViewSd(bool start) const
{
    const std::uint8_t val = static_cast<std::uint8_t>(start ? 0x01U : 0x00U);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x93U, 0x00U, val);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFirstMovieFile(std::uint16_t fileNo) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((fileNo >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(fileNo & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x95U, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildPlayMovieSd(SdPlaybackControl ctrl) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x97U, 0x00U, static_cast<std::uint8_t>(ctrl));
}

std::vector<std::uint8_t> ProtocolBuilder::buildDeleteMovieSd(std::uint16_t fileNo) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((fileNo >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(fileNo & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x99U, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSelectMoviePlayMode(SdPlaybackMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x9BU, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildMenuOk() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xA7U, 0x00U, 0x01U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildMenuDirection(MenuDirection dir) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xA9U, 0x00U, static_cast<std::uint8_t>(dir));
}

std::vector<std::uint8_t> ProtocolBuilder::buildMenuBack() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xABU, 0x00U, 0x01U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildMenuKey(MenuKey key) const
{
    if (key == MenuKey::Ok)
        return buildMenuOk();
    if (key == MenuKey::Back)
        return buildMenuBack();
    if (key == MenuKey::Up)
        return buildMenuDirection(MenuDirection::Up);
    if (key == MenuKey::Down)
        return buildMenuDirection(MenuDirection::Down);
    if (key == MenuKey::Left)
        return buildMenuDirection(MenuDirection::Left);
    if (key == MenuKey::Right)
        return buildMenuDirection(MenuDirection::Right);
    return buildMenuOk();
}

std::vector<std::uint8_t> ProtocolBuilder::buildSdPlaybackControl(std::uint8_t action) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0x97U, 0x00U, action);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryKeySettings(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xADU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.9 Original Command 6 (SD Movie Catalog)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildQuerySdMovieCount() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xB1U, 0xB3U, 0x00U);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQuerySdMovieYear(std::uint16_t fileNo) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((fileNo >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(fileNo & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xB5U, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQuerySdMovieMonthDay(std::uint16_t fileNo) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((fileNo >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(fileNo & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xB9U, d1, d2);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQuerySdMovieHourMin(std::uint16_t fileNo) const
{
    const std::uint8_t d1 = static_cast<std::uint8_t>((fileNo >> 8U) & 0xFFU);
    const std::uint8_t d2 = static_cast<std::uint8_t>(fileNo & 0xFFU);
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xBDU, d1, d2);
}

// -----------------------------------------------------------------------------
// 5.10 Original Command 7 (Log Data)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildQueryLogData(LogType type) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xD1U, 0xD3U, static_cast<std::uint8_t>(type));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryLogData(std::uint8_t type) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xD1U, 0xD3U, type);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryTemperature() const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xC1U, 0x00U, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.11 Original Command 8 (Fine Settings)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetFineBrightness(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xEBU, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFineContrast(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xEDU, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFineSaturation(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xEFU, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFineSharpness(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xF1U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWbRedShift(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xF5U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetWbBlueShift(std::int8_t val) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xF7U, 0x00U, static_cast<std::uint8_t>(val));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryFineSettings(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF0U, 0xFDU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.12 Original Command 9 (Day/Night Extended Settings)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetDayNightMode(DayNightMode mode) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x01U, 0x00U, static_cast<std::uint8_t>(mode));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDayToNightThreshold(std::uint8_t th) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x03U, 0x00U, th);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetNightToDayThreshold(std::uint8_t th) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x05U, 0x00U, th);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDayNightThreshold(std::uint8_t d2n, std::uint8_t n2d) const
{
    (void)n2d;
    return buildSetDayToNightThreshold(d2n);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDayNightAutoDelay(std::uint8_t delay) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x07U, 0x00U, delay);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetDayStartTime(std::uint8_t hour, std::uint8_t min) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x09U, hour, min);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetNightStartTime(std::uint8_t hour, std::uint8_t min) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x0BU, hour, min);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetOpticalFilterDay(OpticalFilter filter) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x0DU, 0x00U, static_cast<std::uint8_t>(filter));
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetOpticalFilterNight(OpticalFilter filter) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x0FU, 0x00U, static_cast<std::uint8_t>(filter));
}

std::vector<std::uint8_t> ProtocolBuilder::buildQueryDayNightEx(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x1DU, target, 0x00U);
}

// -----------------------------------------------------------------------------
// 5.13 Original Command 10 (Zoom/Focus Speed Extended)
// -----------------------------------------------------------------------------
std::vector<std::uint8_t> ProtocolBuilder::buildSetDigitalZoomEx(std::uint8_t mag) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x21U, 0x00U, mag);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetZoomSpeedEx(std::uint8_t speed) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x25U, 0x00U, speed);
}

std::vector<std::uint8_t> ProtocolBuilder::buildSetFocusSpeedEx(std::uint8_t speed) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x27U, 0x00U, speed);
}

std::vector<std::uint8_t> ProtocolBuilder::buildQuerySpeedEx(std::uint8_t target) const
{
    return PelcoDFrame::createFrame(m_address, 0xF1U, 0x2DU, target, 0x00U);
}

} // namespace FujinonSX800

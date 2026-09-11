#pragma once

/// @file ProtocolBuilder.h
/// @brief Pelco-D frame serializer covering 100% of the Fujinon SX800 specification v2.12.0.

#include "PelcoDFrame.h"
#include "PelcoDTypes.h"

#include <cstdint>
#include <vector>

namespace FujinonSX800 {

/// @class ProtocolBuilder
/// @brief Serializes high-level camera commands into standard 7-byte Pelco-D wire frames.
class ProtocolBuilder {
public:
    explicit ProtocolBuilder(std::uint8_t cameraAddress = 1U) noexcept;

    void setAddress(std::uint8_t address) noexcept;
    [[nodiscard]] std::uint8_t getAddress() const noexcept;

    // -------------------------------------------------------------------------
    // 5.1 Standard Commands
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildZoom(ZoomDirection dir) const;
    [[nodiscard]] std::vector<std::uint8_t> buildFocus(FocusDirection dir) const;
    [[nodiscard]] std::vector<std::uint8_t> buildIris(IrisDirection dir) const;
    [[nodiscard]] std::vector<std::uint8_t> buildStop() const;

    // -------------------------------------------------------------------------
    // 5.2 Extended Commands
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetZoomSpeed(std::uint8_t speed) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetFocusSpeed(std::uint8_t speed) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetAutoFocus(AutoFocusMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetAutoIris(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetAgc(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetBlc(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetZoomPosition(std::uint16_t pulse) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryZoomPosition() const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetBaudRate(BaudRate rate) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetRtcTime(
        std::uint8_t year, std::uint8_t month, std::uint8_t day,
        std::uint8_t hour, std::uint8_t minute, std::uint8_t second) const;

    // -------------------------------------------------------------------------
    // 5.3 FF Extended Commands
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildQueryFocusPosition() const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryZoomPositionFf() const;
    [[nodiscard]] std::vector<std::uint8_t> buildQuerySerialNumber() const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryFwVersion() const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryLensStatus() const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetFocusPosition(std::uint16_t pulse) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetManualIris(ManualIrisFNo fNo) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetIrisPosition(std::uint16_t pos) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetShutterLimit(ShutterLimitMode limit) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetShutterSpeed(ShutterSpeed speed) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryShutterSpeed() const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetIso(IsoSensitivity iso) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryIso() const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryIrisPosition() const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryManualSettings(std::uint8_t target = 0x25U) const;

    // -------------------------------------------------------------------------
    // 5.4 Original Command 1 (Photo Settings)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetAfArea(AfArea area) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetAfSensitivity(AfSensitivity sens) const;
    [[nodiscard]] std::vector<std::uint8_t> buildOnePushAf() const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetAutoDayNight(DayNightMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDayNight(DayNightMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetIrWavelength(IrWavelength wl) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetOpticalStabilization(OpticalStabilization mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetPhotoPreset(std::uint8_t preset) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDayNightTrigger(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryPhotoSettings(std::uint8_t target = 0x03U) const;

    // -------------------------------------------------------------------------
    // 5.5 Original Command 2 (Image Quality)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetVlcFilter(VlcFilterMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetWdr(WdrMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDeHeatHaze(DeHeatHazeMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDefog(DefogMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetBrightness(std::uint8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetContrast(std::uint8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetSaturation(std::uint8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetSharpness(std::uint8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetColorTemperature(std::uint16_t kelvin) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetWhiteBalance(WhiteBalanceMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDigitalZoom(DigitalZoomMode mode, std::uint8_t mag) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetNoiseReduction(NoiseReductionLevel level) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryImageQuality(std::uint8_t target = 0x21U) const;

    // -------------------------------------------------------------------------
    // 5.6 Original Command 3 (Display Settings / OSD)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetDateTimeDisplay(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDateTimePosition(OsdPosition pos) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetTitleDisplay(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetTitlePosition(OsdPosition pos) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetIdDisplay(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetIdPosition(OsdPosition pos) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetReticleDisplay(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetAntialiasing(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryDisplaySettings(std::uint8_t target = 0x43U) const;

    // -------------------------------------------------------------------------
    // 5.7 Original Command 4 (Operation Settings / System)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetTimeDisplayFormat(TimeDisplayFormat fmt) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDateDisplayFormat(DateDisplayFormat fmt) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetVideoStandard(VideoStandard standard) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetHdFormat(HdVideoFormat format) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetVideoDisplayMode(VideoDisplayMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetRs485Id(std::uint8_t id) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetTermination(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetSdRecordingMode(SdRecordMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetSdCapacityDisplay(bool enable) const;
    [[nodiscard]] std::vector<std::uint8_t> buildFormatSdCard() const;
    [[nodiscard]] std::vector<std::uint8_t> buildDownloadFirmware() const;
    [[nodiscard]] std::vector<std::uint8_t> buildRecordLogSd() const;
    [[nodiscard]] std::vector<std::uint8_t> buildPreset(PresetAction action, std::uint8_t presetId) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSavePreset(std::uint8_t presetId) const;
    [[nodiscard]] std::vector<std::uint8_t> buildRecallPreset(std::uint8_t presetId) const;
    [[nodiscard]] std::vector<std::uint8_t> buildReboot() const;
    [[nodiscard]] std::vector<std::uint8_t> buildFactoryReset() const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetLanguage(Language lang) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryOperationSettings(std::uint8_t target = 0x61U) const;

    // -------------------------------------------------------------------------
    // 5.8 Original Command 5 (Key Settings & SD Playback)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetEncodeMode(std::uint8_t mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildRecordLiveViewSd(bool start) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetFirstMovieFile(std::uint16_t fileNo) const;
    [[nodiscard]] std::vector<std::uint8_t> buildPlayMovieSd(SdPlaybackControl ctrl) const;
    [[nodiscard]] std::vector<std::uint8_t> buildDeleteMovieSd(std::uint16_t fileNo) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSelectMoviePlayMode(SdPlaybackMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildMenuOk() const;
    [[nodiscard]] std::vector<std::uint8_t> buildMenuDirection(MenuDirection dir) const;
    [[nodiscard]] std::vector<std::uint8_t> buildMenuBack() const;
    [[nodiscard]] std::vector<std::uint8_t> buildMenuKey(MenuKey key) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSdPlaybackControl(std::uint8_t action) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryKeySettings(std::uint8_t target = 0x91U) const;

    // -------------------------------------------------------------------------
    // 5.9 Original Command 6 (SD Movie Catalog)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildQuerySdMovieCount() const;
    [[nodiscard]] std::vector<std::uint8_t> buildQuerySdMovieYear(std::uint16_t fileNo) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQuerySdMovieMonthDay(std::uint16_t fileNo) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQuerySdMovieHourMin(std::uint16_t fileNo) const;

    // -------------------------------------------------------------------------
    // 5.10 Original Command 7 (Log Data)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildQueryLogData(LogType type) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryLogData(std::uint8_t type) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryTemperature() const;

    // -------------------------------------------------------------------------
    // 5.11 Original Command 8 (Fine Settings)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetFineBrightness(std::int8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetFineContrast(std::int8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetFineSaturation(std::int8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetFineSharpness(std::int8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetWbRedShift(std::int8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetWbBlueShift(std::int8_t val) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryFineSettings(std::uint8_t target = 0xEBU) const;

    // -------------------------------------------------------------------------
    // 5.12 Original Command 9 (Day/Night Extended Settings)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetDayNightMode(DayNightMode mode) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDayToNightThreshold(std::uint8_t th) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetNightToDayThreshold(std::uint8_t th) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDayNightThreshold(std::uint8_t d2n, std::uint8_t n2d) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDayNightAutoDelay(std::uint8_t delay) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetDayStartTime(std::uint8_t hour, std::uint8_t min) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetNightStartTime(std::uint8_t hour, std::uint8_t min) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetOpticalFilterDay(OpticalFilter filter) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetOpticalFilterNight(OpticalFilter filter) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQueryDayNightEx(std::uint8_t target = 0x01U) const;

    // -------------------------------------------------------------------------
    // 5.13 Original Command 10 (Zoom/Focus Speed Extended)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<std::uint8_t> buildSetDigitalZoomEx(std::uint8_t mag) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetZoomSpeedEx(std::uint8_t speed) const;
    [[nodiscard]] std::vector<std::uint8_t> buildSetFocusSpeedEx(std::uint8_t speed) const;
    [[nodiscard]] std::vector<std::uint8_t> buildQuerySpeedEx(std::uint8_t target = 0x25U) const;

private:
    std::uint8_t m_address { 1U };
};

} // namespace FujinonSX800

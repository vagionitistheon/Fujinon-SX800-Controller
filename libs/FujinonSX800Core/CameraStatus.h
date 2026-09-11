#pragma once

/// @file CameraStatus.h
/// @brief Telemetry and state representation of the Fujinon SX800 camera.

#include "PelcoDTypes.h"

#include <chrono>
#include <cstdint>
#include <string>

namespace FujinonSX800 {

/// @struct CameraStatus
/// @brief Aggregates all operational parameters and telemetry flags.
struct CameraStatus {
    // ---------- Optics & Motion ----------
    std::uint16_t zoomPosition { 0U };
    double focalLengthMm { 20.0 };
    double horizontalFovDeg { 20.974 };
    double verticalFovDeg { 11.847 };
    double diagonalFovDeg { 24.017 };
    double ifovMrad { 0.203 };
    std::uint8_t zoomSpeed { 5U };
    std::uint8_t zoomSpeedEx { 5U };
    bool zoomMoving { false };
    bool zoomTeleLimit { false };
    bool zoomWideLimit { false };

    std::uint16_t focusPosition { 0U };
    double subjectDistanceM { 0.0 };
    double focusDistanceM { 0.0 };
    std::uint8_t focusSpeed { 5U };
    std::uint8_t focusSpeedEx { 5U };
    bool focusMoving { false };
    bool focusFarLimit { false };
    bool focusNearLimit { false };

    AutoFocusMode autoFocusMode { AutoFocusMode::Off };
    AfArea afArea { AfArea::Center };
    AfSensitivity afSensitivity { AfSensitivity::Normal };

    DigitalZoomMode digitalZoomMode { DigitalZoomMode::Off };
    DigitalZoomStep digitalZoomStep { DigitalZoomStep::X1_00 };
    std::uint8_t digitalZoomMag { 1U };

    // ---------- Exposure & Iris ----------
    AutoIrisMode autoIrisMode { AutoIrisMode::On };
    std::uint16_t manualIrisPosition { 0U };
    std::uint16_t irisPosition { 0U };
    ManualIrisFNo manualIrisFNo { ManualIrisFNo::F4_0 };
    bool irisMoving { false };
    bool irisCloseLimit { false };
    bool irisOpenLimit { false };

    AgcMode agcMode { AgcMode::Auto };
    BlcMode blcMode { BlcMode::Off };
    ShutterLimitMode shutterLimitMode { ShutterLimitMode::Manual };
    std::uint8_t shutterLimitAuto { 0U };
    ShutterSpeed shutterSpeed { ShutterSpeed::Auto };
    std::uint16_t manualShutterSpeed { 0U };
    IsoSensitivity isoSensitivity { IsoSensitivity::Auto };
    std::uint16_t manualIso { 0U };

    // ---------- Image Quality ----------
    std::uint8_t brightness { 10U };
    std::int16_t brightnessFine { 0 };
    std::int8_t fineBrightness { 0 };
    std::uint8_t contrast { 3U };
    std::int16_t contrastFine { 0 };
    std::int8_t fineContrast { 0 };
    std::uint8_t saturation { 3U };
    std::int16_t saturationFine { 0 };
    std::int8_t fineSaturation { 0 };
    std::uint8_t sharpness { 3U };
    std::int16_t sharpnessFine { 0 };
    std::int8_t fineSharpness { 0 };

    VlcFilterMode vlcFilter { VlcFilterMode::Off };
    DefogMode defogMode { DefogMode::Off };
    DefogMode defog { DefogMode::Off };
    DeHeatHazeMode deHeatHazeMode { DeHeatHazeMode::Off };
    DeHeatHazeMode deHeatHaze { DeHeatHazeMode::Off };
    StabilizationMode stabilizationMode { StabilizationMode::Off };
    StabilizationMode opticalStabilization { StabilizationMode::Off };
    WdrMode wdrMode { WdrMode::Off };
    WdrMode wdr { WdrMode::Off };
    NoiseReduction noiseReduction { NoiseReduction::Off };

    WhiteBalanceMode whiteBalanceMode { WhiteBalanceMode::Auto };
    WhiteBalanceMode whiteBalance { WhiteBalanceMode::Auto };
    std::uint16_t colorTempKelvin { 5500U };
    std::uint16_t colorTemperatureKelvin { 5500U };
    std::int16_t wbShiftRedFine { 0 };
    std::int8_t wbRedShift { 0 };
    std::int16_t wbShiftBlueFine { 0 };
    std::int8_t wbBlueShift { 0 };
    DigitalZoomMode digitalZoom { DigitalZoomMode::Off };

    // ---------- Day / Night & Filters ----------
    DayNightMode dayNightMode { DayNightMode::Auto };
    std::uint8_t dayToNightThreshold { 30U };
    std::uint8_t nightToDayThreshold { 50U };
    std::uint8_t dayNightAutoDelaySec { 5U };
    std::uint16_t dayStartTimeMinutes { 360U };   // 06:00
    std::uint16_t nightStartTimeMinutes { 1080U }; // 18:00
    OpticalFilter opticalFilterDay { OpticalFilter::Visible };
    OpticalFilter filterDay { OpticalFilter::Visible };
    OpticalFilter opticalFilterNight { OpticalFilter::IR };
    OpticalFilter filterNight { OpticalFilter::IR };
    IrWavelength irWavelength { IrWavelength::Standard };

    // ---------- OSD & Display ----------
    bool dateTimeDisplay { false };
    bool osdDateTime { false };
    TimeDisplayFormat timeDisplayFormat { TimeDisplayFormat::Hour24 };
    DateDisplayFormat dateDisplayFormat { DateDisplayFormat::YMD };
    OsdPosition dateTimePosition { OsdPosition::TopLeft };

    bool titleDisplay { false };
    bool osdTitle { false };
    OsdPosition titlePosition { OsdPosition::TopRight };
    std::string titleText {};

    bool idDisplay { false };
    bool osdId { false };
    OsdPosition idPosition { OsdPosition::BottomLeft };

    bool centerPositionDisplay { false };
    bool osdReticle { false };
    bool antialiasingEnabled { true };
    bool osdAntialiasing { true };

    // ---------- Video & System ----------
    VideoStandard videoStandard { VideoStandard::NTSC };
    HdFormat hdFormat { HdFormat::HD1080p_60 };
    VideoDisplayMode videoDisplayMode { VideoDisplayMode::Fit };
    std::uint8_t rs485Address { 7U };
    std::uint8_t address { 7U };
    BaudRate baudRate { BaudRate::Baud9600 };
    bool rs485Termination { false };
    bool terminationEnabled { false };
    Language language { Language::English };

    // ---------- SD Card & Recording ----------
    SdRecordMode sdRecordMode { SdRecordMode::FullStop };
    std::uint16_t sdMovieCount { 0U };
    std::uint16_t sdRemainingMinutes { 0U };
    SdPlaybackMode sdPlaybackMode { SdPlaybackMode::LiveView };

    std::string serialNumber {};
    std::uint8_t fwVersionMajor { 0U };
    std::uint8_t fwVersionMinor { 0U };
    double internalTemperatureC { 0.0 };
    bool temperatureValid { false };

    // ---------- State Tracking ----------
    bool isConnected { false };
    bool connected { false };
    std::chrono::steady_clock::time_point lastUpdateTime {};
};

} // namespace FujinonSX800

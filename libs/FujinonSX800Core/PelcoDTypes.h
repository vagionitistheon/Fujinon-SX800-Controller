#pragma once

/// @file PelcoDTypes.h
/// @brief Type definitions and enumerations for Fujinon SX800 Pelco-D protocol.

#include <cstdint>

namespace FujinonSX800 {

/// @enum BaudRate
/// @brief Supported serial baud rates on RS-485 (Section 5.2.9).
enum class BaudRate : std::uint8_t {
    Baud2400 = 0x00U,
    Baud4800 = 0x01U,
    Baud9600 = 0x02U,
    Baud19200 = 0x03U,
    Baud38400 = 0x04U,
    Baud115200 = 0x05U
};

/// @enum ZoomDirection
/// @brief Zoom motion direction (Section 5.1).
enum class ZoomDirection : std::uint8_t { Stop = 0x00U, Tele = 0x01U, Wide = 0x02U };

/// @enum FocusDirection
/// @brief Focus motion direction (Section 5.1).
enum class FocusDirection : std::uint8_t { Stop = 0x00U, Near = 0x01U, Far = 0x02U };

/// @enum IrisDirection
/// @brief Iris movement direction (Section 5.1).
enum class IrisDirection : std::uint8_t { Stop = 0x00U, Open = 0x01U, Close = 0x02U };

/// @enum AutoFocusMode
/// @brief Auto Focus state (Section 5.2.3).
enum class AutoFocusMode : std::uint8_t { On = 0x00U, Off = 0x01U };

/// @enum AutoIrisMode
/// @brief Auto Iris state (Section 5.2.4).
enum class AutoIrisMode : std::uint8_t { On = 0x00U, Off = 0x01U };

/// @enum AgcMode
/// @brief Auto Gain Control mode (Section 5.2.5).
enum class AgcMode : std::uint8_t { Auto = 0x00U, Manual = 0x01U };

/// @enum BlcMode
/// @brief Backlight compensation mode (Section 5.2.6).
enum class BlcMode : std::uint8_t { Off = 0x00U, On = 0x01U };

/// @enum StabilizationMode
/// @brief Image stabilization mode (Section 5.4.9).
enum class StabilizationMode : std::uint8_t { Off = 0x00U, OIS = 0x01U, EIS = 0x02U, Auto = 0x03U };

using OpticalStabilization = StabilizationMode;

/// @enum VlcFilterMode
/// @brief Visible Light Cut filter setting (Section 5.5.1).
enum class VlcFilterMode : std::uint8_t { Off = 0x00U, On = 0x01U, Filter4 = 0x02U };

/// @enum DigitalZoomMode
/// @brief Digital zoom operation mode (Section 5.5.12).
enum class DigitalZoomMode : std::uint8_t { Off = 0x00U, Combined = 0x01U, Crop = 0x02U, Separate = 0x02U };

/// @enum DigitalZoomStep
/// @brief Digital zoom magnification step.
enum class DigitalZoomStep : std::uint8_t { X1_00 = 0x00U, X1_25 = 0x01U, X1_50 = 0x02U, X1_75 = 0x03U, X2_00 = 0x04U };

/// @enum AfArea
/// @brief Auto focus detection area (Section 5.4.1).
enum class AfArea : std::uint8_t {
    Center = 0x00U,
    UpperLeft = 0x01U,
    UpperCenter = 0x02U,
    UpperRight = 0x03U,
    MiddleLeft = 0x04U,
    MiddleCenter = 0x05U,
    MiddleRight = 0x06U,
    LowerLeft = 0x07U,
    LowerCenter = 0x08U,
    LowerRight = 0x09U,
    Area1 = 0x01U,
    Area2 = 0x02U,
    Area3 = 0x03U,
    Area4 = 0x04U,
    Area5 = 0x05U,
    Area6 = 0x06U,
    Area7 = 0x07U,
    Area8 = 0x08U,
    Area9 = 0x09U,
    Full = 0x0AU
};

/// @enum AfSensitivity
/// @brief Auto focus tracking sensitivity (Section 5.4.2).
enum class AfSensitivity : std::uint8_t { Low = 0x01U, Middle = 0x02U, Normal = 0x02U, High = 0x03U };

/// @enum DayNightMode
/// @brief Day and Night switching mode (Section 5.4.6 / 5.4.7 / 5.12.1).
enum class DayNightMode : std::uint8_t { Auto = 0x00U, Day = 0x01U, Night = 0x02U, Schedule = 0x03U, External = 0x04U };

/// @enum OpticalFilter
/// @brief Physical optical filter selection (Section 5.12.7 / 5.12.8).
enum class OpticalFilter : std::uint8_t { Visible = 0x00U, Vlc = 0x01U, IR = 0x02U, Filter4 = 0x03U };

/// @enum IrWavelength
/// @brief Infrared wavelength tuning (Section 5.4.8).
enum class IrWavelength : std::uint8_t {
    VisibleLight = 0x00U,
    Standard = 0x00U,
    Wave950nm = 0x01U,
    Wave940nm = 0x02U,
    Wave850nm = 0x03U,
    Wave808nm = 0x04U
};

/// @enum WdrMode
/// @brief Wide Dynamic Range / Hyper-DNR mode (Section 5.5.2).
enum class WdrMode : std::uint8_t { Off = 0x00U, Low = 0x01U, Mid = 0x02U, High = 0x03U, Auto = 0x04U };

/// @enum DefogMode
/// @brief Optical Defogging level (Section 5.5.5).
enum class DefogMode : std::uint8_t { Off = 0x00U, Low = 0x01U, Mid = 0x02U, High = 0x03U };

/// @enum DeHeatHazeMode
/// @brief De-Heat Haze processing level (Section 5.5.4).
enum class DeHeatHazeMode : std::uint8_t { Off = 0x00U, Low = 0x01U, Mid = 0x02U, High = 0x03U };

/// @enum NoiseReduction
/// @brief 3D Noise Reduction level (Section 5.5.13).
enum class NoiseReduction : std::uint8_t {
    Off = 0x00U,
    Level1 = 0x01U,
    Level2 = 0x02U,
    Level3 = 0x03U,
    Level4 = 0x04U,
    Level5 = 0x05U
};

using NoiseReductionLevel = NoiseReduction;

/// @enum WhiteBalanceMode
/// @brief White balance mode (Section 5.5.11).
enum class WhiteBalanceMode : std::uint8_t {
    Auto = 0x00U,
    Outdoor = 0x01U,
    Indoor = 0x02U,
    OnePush = 0x03U,
    ColorTemp = 0x04U,
    Manual = 0x05U
};

/// @enum ManualIrisFNo
/// @brief Manual iris F-number value (Section 5.3.9 / 5.3.15).
enum class ManualIrisFNo : std::uint8_t {
    F4_0 = 0x01U,
    F4_5 = 0x02U,
    F5_0 = 0x03U,
    F5_6 = 0x04U,
    F6_3 = 0x05U,
    F7_1 = 0x06U,
    F8_0 = 0x07U,
    F9_0 = 0x08U,
    F10 = 0x09U,
    F11 = 0x0AU,
    F13 = 0x0BU,
    F14 = 0x0CU,
    F16 = 0x0DU,
    F18 = 0x0EU,
    F20 = 0x0FU,
    F22 = 0x10U,
    F25 = 0x11U,
    Close = 0x12U
};

/// @enum ShutterLimitMode
/// @brief Shutter lowest limit on auto exposure (Section 5.3.10).
enum class ShutterLimitMode : std::uint8_t {
    Manual = 0x00U,
    Lowest1_8 = 0x01U,
    Lowest1_15 = 0x02U,
    Lowest1_30 = 0x03U,
    Lowest1_60 = 0x04U,
    Lowest1_125 = 0x05U,
    Lowest1_250 = 0x06U,
    Lowest1_500 = 0x07U,
    Lowest1_1000 = 0x08U
};

/// @enum ShutterSpeed
/// @brief Shutter speed settings (Section 5.3.11).
enum class ShutterSpeed : std::uint8_t {
    Auto = 0x00U,
    Speed1_1 = 0x01U,
    Speed1_1_3 = 0x02U,
    Speed1_1_6 = 0x03U,
    Speed1_2 = 0x04U,
    Speed1_2_5 = 0x05U,
    Speed1_3 = 0x06U,
    Speed1_4 = 0x07U,
    Speed1_5 = 0x08U,
    Speed1_6 = 0x09U,
    Speed1_8 = 0x0AU,
    Speed1_10 = 0x0BU,
    Speed1_12 = 0x0CU,
    Speed1_15 = 0x0DU,
    Speed1_20 = 0x0EU,
    Speed1_25 = 0x0FU,
    Speed1_30 = 0x10U,
    Speed1_40 = 0x11U,
    Speed1_50 = 0x12U,
    Speed1_60 = 0x13U,
    Speed1_80 = 0x14U,
    Speed1_100 = 0x15U,
    Speed1_120 = 0x16U,
    Speed1_125 = 0x17U,
    Speed1_160 = 0x18U,
    Speed1_200 = 0x19U,
    Speed1_240 = 0x1AU,
    Speed1_250 = 0x1BU,
    Speed1_320 = 0x1CU,
    Speed1_400 = 0x1DU,
    Speed1_500 = 0x1EU,
    Speed1_800 = 0x1FU,
    Speed1_1000 = 0x20U,
    Speed1_1600 = 0x21U,
    Speed1_2000 = 0x22U,
    Speed1_3200 = 0x23U,
    Speed1_4000 = 0x24U,
    Speed1_6000 = 0x25U,
    Speed1_8000 = 0x26U,
    Speed1_10000 = 0x27U,
    Speed1_16000 = 0x28U,
    Speed1_20000 = 0x29U,
    Speed1_32000 = 0x2AU
};

/// @enum IsoSensitivity
/// @brief ISO sensitivity settings (Section 5.3.13).
enum class IsoSensitivity : std::uint8_t {
    Auto = 0x00U,
    Iso400 = 0x01U,
    Iso500 = 0x02U,
    Iso640 = 0x03U,
    Iso800 = 0x04U,
    Iso1000 = 0x05U,
    Iso1250 = 0x06U,
    Iso1600 = 0x07U,
    Iso2000 = 0x08U,
    Iso2500 = 0x09U,
    Iso3200 = 0x0AU,
    Iso4000 = 0x0BU,
    Iso5000 = 0x0CU,
    Iso6400 = 0x0DU,
    Iso8000 = 0x0EU,
    Iso10000 = 0x0FU,
    Iso12800 = 0x10U,
    Iso25600 = 0x11U,
    Iso51200 = 0x12U,
    Iso102400 = 0x13U,
    Iso204800 = 0x14U,
    Iso409600 = 0x15U,
    Iso819200 = 0x16U
};

/// @enum TimeDisplayFormat
/// @brief OSD time display mode (Section 5.7.1).
enum class TimeDisplayFormat : std::uint8_t { Hour24 = 0x01U, Hour12 = 0x02U };

using TimeDisplayMode = TimeDisplayFormat;

/// @enum DateDisplayFormat
/// @brief OSD date display format (Section 5.7.2).
enum class DateDisplayFormat : std::uint8_t { YMD = 0x01U, MDY = 0x02U, DMY = 0x03U };

using YmdDisplayMode = DateDisplayFormat;

/// @enum OsdPosition
/// @brief OSD element display position (Section 5.6.3 / 5.6.6 / 5.6.8).
enum class OsdPosition : std::uint8_t { TopLeft = 0x01U, TopRight = 0x02U, BottomLeft = 0x03U, BottomRight = 0x04U };

/// @enum VideoStandard
/// @brief Video standard NTSC or PAL (Section 5.7.4).
enum class VideoStandard : std::uint8_t { NTSC = 0x01U, PAL = 0x02U };

/// @enum HdFormat
/// @brief Video output resolution and framerate format (Section 5.7.5).
enum class HdFormat : std::uint8_t {
    HD1080p_60 = 0x01U,
    HD1080p_50 = 0x02U,
    HD1080p_30 = 0x03U,
    HD1080p_25 = 0x04U,
    HD720p_60 = 0x05U,
    HD720p_50 = 0x06U
};

using HdVideoFormat = HdFormat;

/// @enum VideoDisplayMode
/// @brief Video display scaling mode (Section 5.7.6).
enum class VideoDisplayMode : std::uint8_t { Fit = 0x01U, Fill = 0x02U };

/// @enum Language
/// @brief Camera OSD language (Section 5.7.19).
enum class Language : std::uint8_t { English = 0x01U, Japanese = 0x02U, Chinese = 0x03U };

/// @enum SdRecordMode
/// @brief SD card recording mode (Section 5.7.12).
enum class SdRecordMode : std::uint8_t { FullStop = 0x01U, OverWrite = 0x02U };

/// @enum PresetAction
/// @brief Preset action operation (Section 5.7.17).
enum class PresetAction : std::uint8_t { Save = 0x00U, Recall = 0x01U, Delete = 0x02U };

/// @enum MenuKey
/// @brief Virtual OSD remote navigation key (Section 5.8.12 - 5.8.14).
enum class MenuKey : std::uint8_t { Up = 0x01U, Down = 0x02U, Left = 0x03U, Right = 0x04U, Ok = 0x05U, Back = 0x06U };

/// @enum MenuDirection
/// @brief OSD menu cursor direction (Section 5.8.13).
enum class MenuDirection : std::uint8_t { Up = 0x01U, Down = 0x02U, Left = 0x03U, Right = 0x04U };

/// @enum SdPlaybackControl
/// @brief SD playback control action (Section 5.8.4).
enum class SdPlaybackControl : std::uint8_t {
    PlayPause = 0x01U,
    Stop = 0x02U,
    FastForward = 0x03U,
    FastRewind = 0x04U
};

/// @enum SdPlaybackMode
/// @brief Movie playback mode switch (Section 5.8.6).
enum class SdPlaybackMode : std::uint8_t { LiveView = 0x01U, Playback = 0x02U };

/// @enum LogType
/// @brief Camera diagnostic log item type (Section 5.10.1).
enum class LogType : std::uint8_t {
    ZoomPulses = 0x01U,
    FocusPulses = 0x02U,
    PowerOnTime = 0x03U,
    OisDriveTime = 0x04U,
    ThermalCycle = 0x05U,
    ErrorHistory = 0x06U
};

/// @enum CommandSwitch
/// @brief Generic enable/disable switch parameter.
enum class CommandSwitch : std::uint8_t { Off = 0x00U, On = 0x01U };

} // namespace FujinonSX800

#pragma once

/// @file PelcoDTypes.h
/// @brief Type definitions and enumerations for Fujinon SX800 Pelco-D protocol.

#include <cstdint>

namespace FujinonSX800 {

/// @enum BaudRate
/// @brief Supported serial baud rates on RS-485.
enum class BaudRate : std::uint8_t {
    Baud2400 = 0x00U,
    Baud4800 = 0x01U,
    Baud9600 = 0x02U,
    Baud19200 = 0x03U,
    Baud38400 = 0x04U,
    Baud115200 = 0x05U
};

/// @enum ZoomDirection
/// @brief Zoom motion direction.
enum class ZoomDirection : std::uint8_t {
    Stop = 0x00U,
    Tele = 0x01U,
    Wide = 0x02U
};

/// @enum FocusDirection
/// @brief Focus motion direction.
enum class FocusDirection : std::uint8_t {
    Stop = 0x00U,
    Near = 0x01U,
    Far = 0x02U
};

/// @enum IrisDirection
/// @brief Iris movement direction.
enum class IrisDirection : std::uint8_t {
    Stop = 0x00U,
    Open = 0x01U,
    Close = 0x02U
};

/// @enum AutoFocusMode
/// @brief Auto Focus state.
enum class AutoFocusMode : std::uint8_t {
    On = 0x00U,
    Off = 0x01U
};

/// @enum AutoIrisMode
/// @brief Auto Iris state.
enum class AutoIrisMode : std::uint8_t {
    On = 0x00U,
    Off = 0x01U
};

/// @enum AgcMode
/// @brief Auto Gain Control mode.
enum class AgcMode : std::uint8_t {
    Auto = 0x00U,
    Manual = 0x01U
};

/// @enum BlcMode
/// @brief Backlight compensation mode.
enum class BlcMode : std::uint8_t {
    Off = 0x00U,
    On = 0x01U
};

/// @enum StabilizationMode
/// @brief Image stabilization mode.
enum class StabilizationMode : std::uint8_t {
    Off = 0x00U,
    OIS = 0x01U,
    EIS = 0x02U,
    Auto = 0x03U
};

using OpticalStabilization = StabilizationMode;

/// @enum VlcFilterMode
/// @brief Visible Light Cut filter setting.
enum class VlcFilterMode : std::uint8_t {
    Off = 0x00U,
    On = 0x01U,
    Filter4 = 0x02U
};

/// @enum DigitalZoomMode
/// @brief Digital zoom operation mode.
enum class DigitalZoomMode : std::uint8_t {
    Off = 0x00U,
    Combined = 0x01U,
    Separate = 0x02U
};

/// @enum DigitalZoomStep
/// @brief Digital zoom magnification step.
enum class DigitalZoomStep : std::uint8_t {
    X1_00 = 0x00U,
    X1_25 = 0x01U,
    X1_50 = 0x02U,
    X1_75 = 0x03U,
    X2_00 = 0x04U
};

/// @enum AfArea
/// @brief Auto focus detection area.
enum class AfArea : std::uint8_t {
    Center = 0x00U,
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
/// @brief Auto focus tracking sensitivity.
enum class AfSensitivity : std::uint8_t {
    Low = 0x00U,
    Normal = 0x01U,
    High = 0x02U
};

/// @enum DayNightMode
/// @brief Day and Night switching mode.
enum class DayNightMode : std::uint8_t {
    Auto = 0x00U,
    Day = 0x01U,
    Night = 0x02U,
    Schedule = 0x03U,
    External = 0x04U
};

/// @enum OpticalFilter
/// @brief Physical optical filter selection.
enum class OpticalFilter : std::uint8_t {
    Visible = 0x00U,
    Vlc = 0x01U,
    IR = 0x02U,
    Filter4 = 0x03U
};

/// @enum IrWavelength
/// @brief Infrared wavelength tuning.
enum class IrWavelength : std::uint8_t {
    VisibleLight = 0x00U,
    Standard = 0x00U,
    Wave950nm = 0x01U,
    Wave940nm = 0x02U,
    Wave850nm = 0x03U,
    Wave808nm = 0x04U
};

/// @enum WdrMode
/// @brief Wide Dynamic Range / Hyper-DNR mode.
enum class WdrMode : std::uint8_t {
    Off = 0x00U,
    Low = 0x01U,
    Mid = 0x02U,
    High = 0x03U,
    Auto = 0x04U
};

/// @enum DefogMode
/// @brief Optical Defogging level.
enum class DefogMode : std::uint8_t {
    Off = 0x00U,
    Low = 0x01U,
    Mid = 0x02U,
    High = 0x03U
};

/// @enum DeHeatHazeMode
/// @brief De-Heat Haze processing level.
enum class DeHeatHazeMode : std::uint8_t {
    Off = 0x00U,
    Low = 0x01U,
    Mid = 0x02U,
    High = 0x03U
};

/// @enum NoiseReduction
/// @brief 3D Noise Reduction level.
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
/// @brief White balance mode.
enum class WhiteBalanceMode : std::uint8_t {
    Auto = 0x00U,
    Outdoor = 0x01U,
    Indoor = 0x02U,
    OnePush = 0x03U,
    ColorTemp = 0x04U,
    Manual = 0x05U
};

/// @enum ShutterSpeed
/// @brief Shutter speed settings.
enum class ShutterSpeed : std::uint8_t {
    Auto = 0x00U,
    Speed1_1 = 0x01U,
    Speed1_2 = 0x02U,
    Speed1_4 = 0x03U,
    Speed1_8 = 0x04U,
    Speed1_15 = 0x05U,
    Speed1_30 = 0x06U,
    Speed1_60 = 0x07U,
    Speed1_125 = 0x08U,
    Speed1_250 = 0x09U,
    Speed1_500 = 0x0AU,
    Speed1_1000 = 0x0BU,
    Speed1_2000 = 0x0CU,
    Speed1_4000 = 0x0DU,
    Speed1_10000 = 0x0EU
};

/// @enum IsoSensitivity
/// @brief ISO sensitivity settings.
enum class IsoSensitivity : std::uint8_t {
    Auto = 0x00U,
    Iso100 = 0x01U,
    Iso200 = 0x02U,
    Iso400 = 0x03U,
    Iso800 = 0x04U,
    Iso1600 = 0x05U,
    Iso3200 = 0x06U,
    Iso6400 = 0x07U,
    Iso12800 = 0x08U
};

/// @enum TimeDisplayFormat
/// @brief OSD time display mode.
enum class TimeDisplayFormat : std::uint8_t {
    Hour24 = 0x01U,
    Hour12 = 0x02U
};

/// @enum DateDisplayFormat
/// @brief OSD date display format.
enum class DateDisplayFormat : std::uint8_t {
    YMD = 0x01U,
    MDY = 0x02U,
    DMY = 0x03U
};

/// @enum OsdPosition
/// @brief OSD element display position.
enum class OsdPosition : std::uint8_t {
    TopLeft = 0x01U,
    TopRight = 0x02U,
    BottomLeft = 0x03U,
    BottomRight = 0x04U
};

/// @enum VideoStandard
/// @brief Video standard (NTSC or PAL).
enum class VideoStandard : std::uint8_t {
    NTSC = 0x00U,
    PAL = 0x01U
};

/// @enum HdFormat
/// @brief Video output resolution and framerate format.
enum class HdFormat : std::uint8_t {
    HD1080p_60 = 0x00U,
    HD1080p_50 = 0x01U,
    HD1080p_30 = 0x02U,
    HD1080p_25 = 0x03U,
    HD720p_60 = 0x04U,
    HD720p_50 = 0x05U
};

using HdVideoFormat = HdFormat;

/// @enum Language
/// @brief Camera OSD language.
enum class Language : std::uint8_t {
    English = 0x00U,
    Japanese = 0x01U,
    Chinese = 0x02U
};

/// @enum SdRecordMode
/// @brief SD card recording mode.
enum class SdRecordMode : std::uint8_t {
    Manual = 0x00U,
    FullStop = 0x01U,
    OverWrite = 0x02U
};

/// @enum MenuKey
/// @brief Virtual OSD remote navigation key.
enum class MenuKey : std::uint8_t {
    Up = 0x01U,
    Down = 0x02U,
    Left = 0x03U,
    Right = 0x04U,
    Ok = 0x05U,
    Back = 0x06U
};

using MenuDirection = MenuKey;

/// @enum LogType
/// @brief Camera diagnostic log item type.
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
enum class CommandSwitch : std::uint8_t {
    Off = 0x00U,
    On = 0x01U
};

} // namespace FujinonSX800

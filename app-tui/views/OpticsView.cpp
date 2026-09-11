#include "OpticsView.h"
#include "Color.h"
#include "UtfSymbols.h"

#include <iomanip>
#include <sstream>

namespace FujinonSX800Tui {

void OpticsView::render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom)
{
    const int w = canvas.width();
    const int availableHeight = bottom - top;

    Style boxStyle {};
    boxStyle.fg = Palette::Border;
    boxStyle.bg = Palette::DarkBg;

    Style titleStyle {};
    titleStyle.fg = Palette::AccentCyan;
    titleStyle.bg = Palette::DarkBg;
    titleStyle.bold = true;

    Style labelStyle {};
    labelStyle.fg = Palette::TextSecondary;
    labelStyle.bg = Palette::DarkBg;

    Style valueStyle {};
    valueStyle.fg = Palette::TextPrimary;
    valueStyle.bg = Palette::DarkBg;
    valueStyle.bold = true;

    Style highlightStyle {};
    highlightStyle.fg = Palette::AccentAmber;
    highlightStyle.bg = Palette::DarkBg;
    highlightStyle.bold = true;

    const int colW = (w - 6) / 2;

    // ==================== 1. ZOOM & FOV PANEL (LEFT) ====================
    canvas.drawBox(2, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(4, top, " [1] OPTICS & GEOMETRY ", titleStyle);

    // Zoom Gauge
    canvas.drawString(4, top + 2, "Optical Zoom Position:", labelStyle);

    std::ostringstream ssZoom;
    const double mag = status.focalLengthMm / 20.0;
    ssZoom << std::fixed << std::setprecision(1) << status.focalLengthMm << " mm (" << std::setprecision(1) << mag
           << "x) [0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << status.zoomPosition << "]";
    canvas.drawString(4, top + 3, ssZoom.str(), valueStyle);

    const double zoomFraction = static_cast<double>(status.zoomPosition) / 16384.0;
    const int barWidth = std::max(10, colW - 6);
    canvas.drawHorizontalBar(4, top + 4, barWidth, zoomFraction, Palette::AccentBlue, Palette::CardBg, Palette::Border,
        Palette::CardBg, true);

    // Zoom Status Badges
    std::string zoomMotion
        = status.zoomMoving ? std::string(Symbols::DotActive) + " MOVING" : std::string(Symbols::DotIdle) + " Idle";
    Style zoomMotionStyle = status.zoomMoving ? highlightStyle : valueStyle;
    canvas.drawString(4, top + 6, "Motion: ", labelStyle);
    canvas.drawString(12, top + 6, zoomMotion, zoomMotionStyle);

    std::string limitStr = "Normal";
    if (status.zoomTeleLimit) {
        limitStr = std::string(Symbols::ArrowUp) + " Tele Limit";
    } else if (status.zoomWideLimit) {
        limitStr = std::string(Symbols::ArrowDown) + " Wide Limit";
    }
    canvas.drawString(24, top + 6, "Limits: ", labelStyle);
    canvas.drawString(
        32, top + 6, limitStr, (status.zoomTeleLimit || status.zoomWideLimit) ? highlightStyle : valueStyle);

    // Optical Geometry Card
    canvas.drawString(4, top + 8, "Calibrated Optical Geometry:", labelStyle);

    auto formatVal = [](double v, const char* unit) {
        std::ostringstream s;
        s << std::fixed << std::setprecision(3) << v << " " << unit;
        return s.str();
    };

    canvas.drawString(6, top + 10, "Horizontal FOV (HFOV):", labelStyle);
    canvas.drawString(30, top + 10, formatVal(status.horizontalFovDeg, "deg"), valueStyle);

    canvas.drawString(6, top + 11, "Vertical FOV (VFOV):", labelStyle);
    canvas.drawString(30, top + 11, formatVal(status.verticalFovDeg, "deg"), valueStyle);

    canvas.drawString(6, top + 12, "Diagonal FOV (DFOV):", labelStyle);
    canvas.drawString(30, top + 12, formatVal(status.diagonalFovDeg, "deg"), valueStyle);

    canvas.drawString(6, top + 13, "IFOV (Instantaneous):", labelStyle);
    canvas.drawString(30, top + 13, formatVal(status.ifovMrad, "mrad"), valueStyle);

    // Digital Zoom
    std::string dZoomStr
        = (status.digitalZoomMode != FujinonSX800::DigitalZoomMode::Off) ? "ON (x1.00 .. x4.00)" : "OFF";
    canvas.drawString(4, top + 15, "Digital Zoom: ", labelStyle);
    canvas.drawString(18, top + 15, dZoomStr, valueStyle);

    // ==================== 2. FOCUS & IRIS PANEL (RIGHT) ====================
    const int rightX = 4 + colW;
    canvas.drawBox(rightX, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(rightX + 2, top, " [2] FOCUS & IRIS ", titleStyle);

    // Focus Gauge
    canvas.drawString(rightX + 2, top + 2, "Focus Position & Distance:", labelStyle);

    std::ostringstream ssFocus;
    ssFocus << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << status.focusPosition
            << " (Dist: " << std::dec << std::fixed << std::setprecision(1) << status.focusDistanceM << " m)";
    canvas.drawString(rightX + 2, top + 3, ssFocus.str(), valueStyle);

    const double focusFraction = static_cast<double>(status.focusPosition) / 65535.0;
    canvas.drawHorizontalBar(rightX + 2, top + 4, barWidth, focusFraction, Palette::AccentCyan, Palette::CardBg,
        Palette::Border, Palette::CardBg, true);

    std::string focusMotion
        = status.focusMoving ? std::string(Symbols::DotActive) + " MOVING" : std::string(Symbols::DotIdle) + " Idle";
    canvas.drawString(rightX + 2, top + 6, "Motion: ", labelStyle);
    canvas.drawString(rightX + 10, top + 6, focusMotion, status.focusMoving ? highlightStyle : valueStyle);

    const char* afNames[] = { "Off (Manual)", "Continuous Auto", "One-Push AF" };
    const int afIdx = static_cast<int>(status.autoFocusMode);
    const char* afStr = (afIdx >= 0 && afIdx < 3) ? afNames[afIdx] : "Unknown";
    canvas.drawString(rightX + 22, top + 6, "AF Mode: ", labelStyle);
    canvas.drawString(rightX + 31, top + 6, afStr, valueStyle);

    // Iris & Exposure Section
    canvas.drawString(rightX + 2, top + 8, "Iris & Exposure Controls:", labelStyle);

    std::string autoIrisStr = (status.autoIrisMode == FujinonSX800::AutoIrisMode::On) ? "AUTO" : "MANUAL";
    canvas.drawString(rightX + 4, top + 10, "Iris Mode:", labelStyle);
    canvas.drawString(rightX + 24, top + 10, autoIrisStr, valueStyle);

    const char* fNoNames[] = { "Close", "F4.0", "F4.5", "F5.0", "F5.6", "F8.0", "F11", "F16", "F22" };
    const int fNoIdx = static_cast<int>(status.manualIrisFNo);
    const char* fNoStr = (fNoIdx >= 0 && fNoIdx < 9) ? fNoNames[fNoIdx] : "F4.0";
    canvas.drawString(rightX + 4, top + 11, "Manual Iris F-No:", labelStyle);
    canvas.drawString(rightX + 24, top + 11, fNoStr, valueStyle);

    std::ostringstream ssIrisPos;
    ssIrisPos << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << status.irisPosition;
    canvas.drawString(rightX + 4, top + 12, "Raw Iris Pos:", labelStyle);
    canvas.drawString(rightX + 24, top + 12, ssIrisPos.str(), valueStyle);

    const char* shutLimits[] = { "Off", "1/8s", "1/15s", "1/30s", "1/60s", "1/125s", "1/250s", "1/500s", "1/1000s" };
    const int shutIdx = static_cast<int>(status.shutterLimitMode);
    const char* shutStr = (shutIdx >= 0 && shutIdx < 9) ? shutLimits[shutIdx] : "Manual";
    canvas.drawString(rightX + 4, top + 13, "Auto Shutter Limit:", labelStyle);
    canvas.drawString(rightX + 24, top + 13, shutStr, valueStyle);

    // Quick control hints
    Style hintStyle {};
    hintStyle.fg = Palette::TextMuted;
    hintStyle.bg = Palette::DarkBg;
    canvas.drawString(
        rightX + 2, top + 15, "Hotkeys: [Z] Zoom In  [X] Zoom Out  [F] Focus Near  [G] Focus Far", hintStyle);
}

} // namespace FujinonSX800Tui

#include "SystemView.h"
#include "Color.h"
#include "UtfSymbols.h"

#include <iomanip>
#include <sstream>

namespace FujinonSX800Tui {

void SystemView::render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom)
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

    const int colW = (w - 6) / 2;

    // ==================== 1. HARDWARE IDENTIFICATION & TELEMETRY (LEFT) ====================
    canvas.drawBox(2, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(4, top, " [1] HARDWARE IDENTITY & TELEMETRY ", titleStyle);

    canvas.drawString(4, top + 2, "Hardware Serial No:", labelStyle);
    std::string serialStr = status.serialNumber.empty() ? "N/A" : status.serialNumber;
    canvas.drawString(24, top + 2, serialStr, valueStyle);

    canvas.drawString(4, top + 4, "Firmware Revision:", labelStyle);
    std::string fwStr = "v" + std::to_string(status.fwVersionMajor) + "." + std::to_string(status.fwVersionMinor);
    canvas.drawString(24, top + 4, fwStr, valueStyle);

    canvas.drawString(4, top + 6, "RS-485 Device Address:", labelStyle);
    canvas.drawString(24, top + 6, std::to_string(status.rs485Address), valueStyle);

    canvas.drawString(4, top + 8, "Lens/Chassis Temp:", labelStyle);
    if (status.temperatureValid) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << status.internalTemperatureC << " °C";
        canvas.drawString(24, top + 8, ss.str(), valueStyle);
    } else {
        canvas.drawString(24, top + 8, "--.- °C", valueStyle);
    }

    // Mechanism Diagnostics
    canvas.drawString(4, top + 11, "Mechanism Diagnostics:", labelStyle);
    canvas.drawString(6, top + 12, "Zoom Motor:   " + std::string(status.zoomMoving ? "MOVING" : "Idle"), valueStyle);
    canvas.drawString(6, top + 13, "Focus Motor:  " + std::string(status.focusMoving ? "MOVING" : "Idle"), valueStyle);
    canvas.drawString(6, top + 14, "Iris Motor:   " + std::string(status.irisMoving ? "MOVING" : "Idle"), valueStyle);

    // ==================== 2. PRESETS & MAINTENANCE (RIGHT) ====================
    const int rightX = 4 + colW;
    canvas.drawBox(rightX, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(rightX + 2, top, " [2] PHOTO PRESETS & MAINTENANCE ", titleStyle);

    canvas.drawString(rightX + 4, top + 2, "Configured Presets (1 - 5):", labelStyle);
    for (int p = 1; p <= 5; ++p) {
        std::string pStr = "Preset " + std::to_string(p) + ":   [Recall]   [Save]   [Delete]";
        canvas.drawString(rightX + 6, top + 3 + p, pStr, valueStyle);
    }

    canvas.drawString(rightX + 4, top + 10, "RS-485 Baud Rate:", labelStyle);
    canvas.drawString(rightX + 26, top + 10, "9600 bps", valueStyle);

    canvas.drawString(rightX + 4, top + 12, "Maintenance Actions:", labelStyle);
    canvas.drawString(rightX + 6, top + 13, "[Reboot Camera]     [Factory Reset]", valueStyle);
}

} // namespace FujinonSX800Tui

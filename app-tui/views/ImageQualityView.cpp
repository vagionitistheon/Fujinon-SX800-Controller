#include "ImageQualityView.h"
#include "Color.h"
#include "UtfSymbols.h"

#include <string>

namespace FujinonSX800Tui {

void ImageQualityView::render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom)
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

    // ==================== 1. TONE & COLOR CALIBRATION (LEFT) ====================
    canvas.drawBox(2, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(4, top, " [1] TONE & FINE SETTINGS ", titleStyle);

    auto drawParamBar = [&](int row, std::string_view name, int val, int minVal, int maxVal) {
        canvas.drawString(4, row, name, labelStyle);
        const std::string valStr = std::to_string(val);
        canvas.drawString(20, row, valStr, valueStyle);

        const double frac = static_cast<double>(val - minVal) / static_cast<double>(maxVal - minVal);
        canvas.drawHorizontalBar(
            26, row, colW - 30, frac, Palette::AccentBlue, Palette::CardBg, Palette::Border, Palette::CardBg, true);
    };

    drawParamBar(top + 2, "Brightness:", status.brightness, -5, 5);
    drawParamBar(top + 4, "Contrast:", status.contrast, -5, 5);
    drawParamBar(top + 6, "Saturation:", status.saturation, -5, 5);
    drawParamBar(top + 8, "Sharpness:", status.sharpness, -5, 5);

    canvas.drawString(4, top + 10, "Fine Color Offsets:", labelStyle);
    drawParamBar(top + 12, "WB Red Shift:", status.wbShiftRedFine, -5, 5);
    drawParamBar(top + 14, "WB Blue Shift:", status.wbShiftBlueFine, -5, 5);

    // ==================== 2. ENHANCEMENT & PROCESSING (RIGHT) ====================
    const int rightX = 4 + colW;
    canvas.drawBox(rightX, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(rightX + 2, top, " [2] PROCESSING & WHITE BALANCE ", titleStyle);

    auto drawToggle = [&](int row, std::string_view name, bool enabled) {
        canvas.drawString(rightX + 4, row, name, labelStyle);
        Style stateStyle = valueStyle;
        if (enabled) {
            stateStyle.fg = Palette::AccentGreen;
            stateStyle.bold = true;
            canvas.drawString(rightX + 26, row, "[ACTIVE]", stateStyle);
        } else {
            stateStyle.fg = Palette::TextMuted;
            canvas.drawString(rightX + 26, row, "[OFF]", stateStyle);
        }
    };

    drawToggle(top + 2, "Wide Dynamic Range (WDR):", status.wdrMode != FujinonSX800::WdrMode::Off);
    drawToggle(top + 4, "Electronic Defog:", status.defogMode != FujinonSX800::DefogMode::Off);
    drawToggle(top + 6, "De-Heat Haze (Atmosphere):", status.deHeatHazeMode != FujinonSX800::DeHeatHazeMode::Off);
    drawToggle(top + 8,
        "Optical Image Stabilization:", status.opticalStabilization != FujinonSX800::OpticalStabilization::Off);
    drawToggle(top + 10, "Visible Light Cut (VLC):", status.vlcFilter != FujinonSX800::VlcFilterMode::Off);

    // White Balance
    canvas.drawString(rightX + 4, top + 12, "White Balance Mode:", labelStyle);
    const char* wbNames[] = { "Auto", "Custom", "Outdoor", "Indoor", "One-Push" };
    const int wbIdx = static_cast<int>(status.whiteBalanceMode);
    const char* wbStr = (wbIdx >= 0 && wbIdx < 5) ? wbNames[wbIdx] : "Auto";
    canvas.drawString(rightX + 26, top + 12, wbStr, valueStyle);

    canvas.drawString(rightX + 4, top + 14, "Color Temperature:", labelStyle);
    std::string kelvinStr = std::to_string(status.colorTemperatureKelvin) + " K";
    canvas.drawString(rightX + 26, top + 14, kelvinStr, valueStyle);
}

} // namespace FujinonSX800Tui

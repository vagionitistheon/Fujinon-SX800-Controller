#include "DayNightView.h"
#include "Color.h"
#include "UtfSymbols.h"

namespace FujinonSX800Tui {

void DayNightView::render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom)
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

    // ==================== 1. MODE & OPTICAL FILTERS (LEFT) ====================
    canvas.drawBox(2, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(4, top, " [1] DAY / NIGHT & OPTICAL BAND ", titleStyle);

    // Current State
    canvas.drawString(4, top + 2, "Operational Day/Night State:", labelStyle);
    Style dnStateStyle = valueStyle;
    std::string dnStr;
    if (status.dayNightMode == FujinonSX800::DayNightMode::Night) {
        dnStateStyle.fg = Palette::AccentPurple;
        dnStr = std::string(Symbols::IconMoon) + " NIGHT MODE";
    } else {
        dnStateStyle.fg = Palette::AccentAmber;
        dnStr = std::string(Symbols::IconSun) + " DAY MODE";
    }
    canvas.drawString(4, top + 3, dnStr, dnStateStyle);

    // Auto Switching
    canvas.drawString(4, top + 5, "Auto Day/Night Control:", labelStyle);
    const char* autoDnStr = (status.dayNightMode == FujinonSX800::DayNightMode::Day) ? "MANUAL DAY"
        : (status.dayNightMode == FujinonSX800::DayNightMode::Night)                 ? "MANUAL NIGHT"
        : (status.dayNightMode == FujinonSX800::DayNightMode::Schedule)              ? "SCHEDULE"
        : (status.dayNightMode == FujinonSX800::DayNightMode::External)              ? "EXTERNAL"
                                                                                     : "AUTOMATIC";
    canvas.drawString(4, top + 6, autoDnStr, valueStyle);

    // Optical Filters
    canvas.drawString(4, top + 8, "Day Optical Filter:", labelStyle);
    std::string dayFilter
        = (status.opticalFilterDay == FujinonSX800::OpticalFilter::IR) ? "IR Filter" : "Visible Light";
    canvas.drawString(24, top + 8, dayFilter, valueStyle);

    canvas.drawString(4, top + 10, "Night Optical Filter:", labelStyle);
    std::string nightFilter
        = (status.opticalFilterNight == FujinonSX800::OpticalFilter::Visible) ? "Visible Light" : "IR Filter";
    canvas.drawString(24, top + 10, nightFilter, valueStyle);

    canvas.drawString(4, top + 12, "Auto Switching Thresholds:", labelStyle);
    std::string trigStr = "D->N: " + std::to_string(status.dayToNightThreshold)
        + " | N->D: " + std::to_string(status.nightToDayThreshold);
    canvas.drawString(28, top + 12, trigStr, valueStyle);

    // ==================== 2. IR SENSITIVITY & WAVELENGTH (RIGHT) ====================
    const int rightX = 4 + colW;
    canvas.drawBox(rightX, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(rightX + 2, top, " [2] INFRARED WAVELENGTH CALIBRATION ", titleStyle);

    canvas.drawString(rightX + 4, top + 2, "Tuned IR Band Wavelength:", labelStyle);
    const char* waveNames[]
        = { "Visible Light (0x00)", "950 nm (0x01)", "940 nm (0x02)", "850 nm (0x03)", "808 nm (0x04)" };
    const int waveIdx = static_cast<int>(status.irWavelength);
    const char* waveStr = (waveIdx >= 0 && waveIdx < 5) ? waveNames[waveIdx] : "950 nm";
    canvas.drawString(rightX + 4, top + 3, waveStr, valueStyle);

    canvas.drawString(rightX + 4, top + 6, "Filter Transmission Spectrum:", labelStyle);
    canvas.drawString(rightX + 6, top + 8, "808 nm  [Laser / Illuminator Target]", labelStyle);
    canvas.drawString(rightX + 6, top + 9, "850 nm  [Standard Semi-Covert IR]", labelStyle);
    canvas.drawString(rightX + 6, top + 10, "940 nm  [Covert Surveillance Band]", labelStyle);
    canvas.drawString(rightX + 6, top + 11, "950 nm  [Deep IR Band Cut]", labelStyle);

    Style noteStyle {};
    noteStyle.fg = Palette::TextMuted;
    noteStyle.bg = Palette::DarkBg;
    canvas.drawString(rightX + 4, top + 14, "Note: SX800 physical motorized filter engages in IR modes.", noteStyle);
}

} // namespace FujinonSX800Tui

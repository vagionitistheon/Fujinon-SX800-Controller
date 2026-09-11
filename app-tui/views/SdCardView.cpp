#include "SdCardView.h"
#include "Color.h"
#include "UtfSymbols.h"

#include <string>

namespace FujinonSX800Tui {

void SdCardView::render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom)
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

    // ==================== 1. STORAGE & RECORDING STATUS (LEFT) ====================
    canvas.drawBox(2, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(4, top, " [1] STORAGE & RECORDING ", titleStyle);

    canvas.drawString(4, top + 2, "SD Recording Mode:", labelStyle);
    const char* recModeStr
        = (status.sdRecordMode == FujinonSX800::SdRecordMode::OverWrite) ? "ENDLESS (Overwrite)" : "FULL-STOP";
    canvas.drawString(24, top + 2, recModeStr, valueStyle);

    canvas.drawString(4, top + 4, "Total Movie Files:", labelStyle);
    canvas.drawString(24, top + 4, std::to_string(status.sdMovieCount) + " clips", valueStyle);

    canvas.drawString(4, top + 6, "Est. Remaining Capacity:", labelStyle);
    canvas.drawString(24, top + 6, std::to_string(status.sdRemainingMinutes) + " min", valueStyle);

    const double capacityFraction = 0.65; // Visual sample gauge
    canvas.drawHorizontalBar(4, top + 8, colW - 8, capacityFraction, Palette::AccentGreen, Palette::CardBg,
        Palette::Border, Palette::CardBg, true);

    // ==================== 2. MEDIA PLAYBACK TRANSPORT (RIGHT) ====================
    const int rightX = 4 + colW;
    canvas.drawBox(rightX, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(rightX + 2, top, " [2] MEDIA PLAYBACK TRANSPORT ", titleStyle);

    canvas.drawString(rightX + 4, top + 2, "Current Playback Mode:", labelStyle);
    const char* playModeStr
        = (status.sdPlaybackMode == FujinonSX800::SdPlaybackMode::Playback) ? "PLAYBACK" : "LIVE VIEW";
    canvas.drawString(rightX + 28, top + 2, playModeStr, valueStyle);

    canvas.drawString(rightX + 4, top + 5, "Media Controls:", labelStyle);
    canvas.drawString(rightX + 6, top + 7,
        "[◀◀ Rewind]   [▶ Play]   [⏸ Pause]   [⏹ Stop]   [FastForward ▶▶]", valueStyle);

    Style hintStyle {};
    hintStyle.fg = Palette::TextMuted;
    hintStyle.bg = Palette::DarkBg;
    canvas.drawString(rightX + 4, top + 10, "Select clip number on camera OSD to begin playback.", hintStyle);
}

} // namespace FujinonSX800Tui

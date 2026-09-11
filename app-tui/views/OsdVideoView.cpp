#include "OsdVideoView.h"
#include "Color.h"
#include "UtfSymbols.h"

namespace FujinonSX800Tui {

void OsdVideoView::render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom)
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

    // ==================== 1. OSD OVERLAYS & POSITIONS (LEFT) ====================
    canvas.drawBox(2, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(4, top, " [1] OSD OVERLAYS & COORDINATES ", titleStyle);

    auto drawOsdRow = [&](int row, std::string_view name, bool enabled, FujinonSX800::OsdPosition pos) {
        canvas.drawString(4, row, name, labelStyle);
        Style st = valueStyle;
        st.fg = enabled ? Palette::AccentGreen : Palette::TextMuted;
        canvas.drawString(22, row, enabled ? "[ON] " : "[OFF]", st);

        const char* pStr = "Top-Left";
        switch (pos) {
        case FujinonSX800::OsdPosition::TopLeft:
            pStr = "Top-Left";
            break;
        case FujinonSX800::OsdPosition::TopRight:
            pStr = "Top-Right";
            break;
        case FujinonSX800::OsdPosition::BottomLeft:
            pStr = "Bottom-Left";
            break;
        case FujinonSX800::OsdPosition::BottomRight:
            pStr = "Bottom-Right";
            break;
        }
        canvas.drawString(30, row, std::string("Pos: ") + pStr, valueStyle);
    };

    drawOsdRow(top + 2, "Date/Time Overlay:", status.dateTimeDisplay, status.dateTimePosition);
    drawOsdRow(top + 4, "Camera Title:", status.titleDisplay, status.titlePosition);
    drawOsdRow(top + 6, "Camera ID Tag:", status.idDisplay, status.idPosition);

    canvas.drawString(4, top + 9, "Reticle Crosshair:", labelStyle);
    canvas.drawString(24, top + 9, status.centerPositionDisplay ? "[ACTIVE]" : "[OFF]", valueStyle);

    canvas.drawString(4, top + 11, "Antialiasing Filter:", labelStyle);
    canvas.drawString(24, top + 11, status.antialiasingEnabled ? "[ENABLED]" : "[DISABLED]", valueStyle);

    // ==================== 2. VIDEO STANDARDS & RTC (RIGHT) ====================
    const int rightX = 4 + colW;
    canvas.drawBox(rightX, top, colW, availableHeight, boxStyle, true, false);
    canvas.drawString(rightX + 2, top, " [2] VIDEO PIPELINE & RTC CLOCK ", titleStyle);

    canvas.drawString(rightX + 4, top + 2, "Analog Video Standard:", labelStyle);
    const char* vidStd = (status.videoStandard == FujinonSX800::VideoStandard::PAL) ? "PAL (50 Hz)" : "NTSC (60 Hz)";
    canvas.drawString(rightX + 28, top + 2, vidStd, valueStyle);

    canvas.drawString(rightX + 4, top + 4, "HD Video Format:", labelStyle);
    const char* hdFmt = (status.hdFormat == FujinonSX800::HdFormat::HD1080p_50) ? "1080p 50fps"
        : (status.hdFormat == FujinonSX800::HdFormat::HD1080p_60)               ? "1080p 60fps"
        : (status.hdFormat == FujinonSX800::HdFormat::HD1080p_30)               ? "1080p 30fps"
        : (status.hdFormat == FujinonSX800::HdFormat::HD1080p_25)               ? "1080p 25fps"
        : (status.hdFormat == FujinonSX800::HdFormat::HD720p_60)                ? "720p 60fps"
        : (status.hdFormat == FujinonSX800::HdFormat::HD720p_50)                ? "720p 50fps"
                                                                                : "1080p (Native)";
    canvas.drawString(rightX + 28, top + 4, hdFmt, valueStyle);

    canvas.drawString(rightX + 4, top + 6, "Video Display Mode:", labelStyle);
    const char* dispMode
        = (status.videoDisplayMode == FujinonSX800::VideoDisplayMode::Fill) ? "FILL (Crop/Stretch)" : "FIT (Letterbox)";
    canvas.drawString(rightX + 28, top + 6, dispMode, valueStyle);

    // Camera Real-Time Clock
    canvas.drawString(rightX + 4, top + 9, "Camera Internal RTC Clock:", labelStyle);
    canvas.drawString(rightX + 4, top + 11, "Status: Synchronized with controller system clock", valueStyle);
}

} // namespace FujinonSX800Tui

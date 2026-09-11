#include "FooterView.h"
#include "Color.h"
#include "UtfSymbols.h"

namespace FujinonSX800Tui {

void FooterView::render(Canvas& canvas, const std::string& statusMessage)
{
    const int w = canvas.width();
    const int h = canvas.height();

    // Line h - 2: Keybinding guide line
    Style guideBgStyle {};
    guideBgStyle.bg = Palette::CardBg;
    guideBgStyle.fg = Palette::TextSecondary;
    canvas.fill(0, h - 2, w, 1, " ", guideBgStyle);

    Style keyStyle {};
    keyStyle.bg = Palette::CardBg;
    keyStyle.fg = Palette::AccentCyan;
    keyStyle.bold = true;

    Style descStyle {};
    descStyle.bg = Palette::CardBg;
    descStyle.fg = Palette::TextPrimary;

    Style sepStyle {};
    sepStyle.bg = Palette::CardBg;
    sepStyle.fg = Palette::Border;

    int curX = 2;
    auto addShortcut = [&](std::string_view key, std::string_view desc) {
        canvas.drawString(curX, h - 2, key, keyStyle);
        curX += static_cast<int>(key.size());
        canvas.drawString(curX, h - 2, " ", descStyle);
        curX++;
        canvas.drawString(curX, h - 2, desc, descStyle);
        curX += static_cast<int>(desc.size());
        canvas.drawString(curX, h - 2, "  │  ", sepStyle);
        curX += 5;
    };

    addShortcut("[1-7]", "Tabs");
    addShortcut("[Tab]", "Next Tab");
    addShortcut("[Z/X]", "Zoom In/Out");
    addShortcut("[F/G]", "Focus Near/Far");
    addShortcut("[Space]", "Stop Motion");
    addShortcut("[C]", "Connect");
    addShortcut("[Q]", "Quit");

    // Line h - 1: Status ticker line
    Style statusBgStyle {};
    statusBgStyle.bg = Palette::PanelBg;
    statusBgStyle.fg = Palette::TextMuted;
    canvas.fill(0, h - 1, w, 1, " ", statusBgStyle);

    Style tickerPrefixStyle {};
    tickerPrefixStyle.bg = Palette::PanelBg;
    tickerPrefixStyle.fg = Palette::AccentBlue;
    tickerPrefixStyle.bold = true;
    canvas.drawString(2, h - 1, "STATUS ▶ ", tickerPrefixStyle);

    Style msgStyle {};
    msgStyle.bg = Palette::PanelBg;
    msgStyle.fg = Palette::TextPrimary;
    const std::string displayMsg = statusMessage.empty() ? "System operational - Telemetry active" : statusMessage;
    canvas.drawString(11, h - 1, displayMsg, msgStyle);
}

} // namespace FujinonSX800Tui

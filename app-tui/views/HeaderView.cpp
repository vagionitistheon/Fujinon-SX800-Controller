#include "HeaderView.h"
#include "Color.h"
#include "UtfSymbols.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace FujinonSX800Tui {

void HeaderView::render(Canvas& canvas, const FujinonSX800::CameraStatus& status, bool isConnected, bool isMock,
    const std::string& connInfo, int activeTab)
{
    const int w = canvas.width();

    // Line 0: Top banner background
    Style headerBgStyle {};
    headerBgStyle.bg = Palette::PanelBg;
    headerBgStyle.fg = Palette::TextSecondary;
    canvas.fill(0, 0, w, 2, " ", headerBgStyle);

    // Title & Icon
    Style titleStyle {};
    titleStyle.bg = Palette::PanelBg;
    titleStyle.fg = Palette::TextPrimary;
    titleStyle.bold = true;
    canvas.drawString(2, 0, "FUJINON SX800 CONTROLLER", titleStyle);

    Style subTitleStyle {};
    subTitleStyle.bg = Palette::PanelBg;
    subTitleStyle.fg = Palette::AccentCyan;
    canvas.drawString(28, 0, "[Pelco-D v2.12.0]", subTitleStyle);

    // Connection Status
    Style connStyle {};
    connStyle.bg = Palette::PanelBg;
    if (isConnected) {
        connStyle.fg = Palette::AccentGreen;
        connStyle.bold = true;
        std::string connText = std::string(Symbols::IconConnected) + " ONLINE";
        if (isMock) {
            connText += " [SIMULATION]";
        } else if (!connInfo.empty()) {
            connText += " (" + connInfo + ")";
        }
        canvas.drawString(48, 0, connText, connStyle);
    } else {
        connStyle.fg = Palette::AccentRed;
        canvas.drawString(48, 0, std::string(Symbols::IconDisconnected) + " DISCONNECTED", connStyle);
    }

    // Telemetry: Lens Temperature
    Style tempStyle {};
    tempStyle.bg = Palette::PanelBg;
    if (status.temperatureValid) {
        const double t = status.internalTemperatureC;
        if (t < 45.0) {
            tempStyle.fg = Palette::AccentGreen;
        } else if (t < 65.0) {
            tempStyle.fg = Palette::AccentAmber;
        } else {
            tempStyle.fg = Palette::AccentRed;
            tempStyle.bold = true;
        }
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << t << " °C";
        canvas.drawString(w - 28, 0, std::string(Symbols::IconTemp) + " " + ss.str(), tempStyle);

        // Mini bar for temperature (0 - 80 C)
        const double frac = (t - 10.0) / 70.0;
        canvas.drawHorizontalBar(
            w - 16, 0, 8, frac, tempStyle.fg, Palette::PanelBg, Palette::Border, Palette::PanelBg, true);
    } else {
        tempStyle.fg = Palette::TextMuted;
        canvas.drawString(w - 28, 0, std::string(Symbols::IconTemp) + " --.- °C", tempStyle);
    }

    // Line 1: Clock
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf {};
#ifndef _WIN32
    ::localtime_r(&nowTime, &tmBuf);
#else
    ::localtime_s(&tmBuf, &nowTime);
#endif
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tmBuf);

    Style clockStyle {};
    clockStyle.bg = Palette::PanelBg;
    clockStyle.fg = Palette::TextSecondary;
    canvas.drawString(w - 10, 0, timeStr, clockStyle);

    // Line 2: Tab Bar
    const char* tabs[] = { "[1] Optics", "[2] Image Quality", "[3] Day/Night", "[4] OSD/Video", "[5] SD Card",
        "[6] System", "[7] Traffic" };

    Style tabLineStyle {};
    tabLineStyle.bg = Palette::CardBg;
    tabLineStyle.fg = Palette::Border;
    canvas.fill(0, 2, w, 1, Symbols::BoxH, tabLineStyle);

    int tabX = 2;
    for (int i = 0; i < 7; ++i) {
        Style tStyle {};
        if (i == activeTab) {
            tStyle.bg = Palette::AccentBlue;
            tStyle.fg = Palette::TextPrimary;
            tStyle.bold = true;
        } else {
            tStyle.bg = Palette::CardBg;
            tStyle.fg = Palette::TextSecondary;
        }

        const std::string label = std::string(" ") + tabs[i] + " ";
        canvas.drawString(tabX, 2, label, tStyle);
        tabX += static_cast<int>(label.size()) + 1;
    }
}

} // namespace FujinonSX800Tui

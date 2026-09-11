#include "TrafficView.h"
#include "Color.h"
#include "FujinonSX800Core/ProtocolParser.h"
#include "UtfSymbols.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace FujinonSX800Tui {

void TrafficView::addFrame(const std::vector<std::uint8_t>& frame, bool isTx)
{
    if (frame.empty()) {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf {};
#ifndef _WIN32
    ::localtime_r(&nowTime, &tmBuf);
#else
    ::localtime_s(&tmBuf, &nowTime);
#endif

    std::ostringstream ssTime;
    ssTime << std::put_time(&tmBuf, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();

    std::ostringstream ssHex;
    for (size_t i = 0; i < frame.size(); ++i) {
        if (i > 0) {
            ssHex << ' ';
        }
        ssHex << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(frame[i]);
    }

    LoggedFrame lf {};
    lf.timestamp = ssTime.str();
    lf.isTx = isTx;
    lf.rawBytes = frame;
    lf.hexStr = ssHex.str();
    lf.description = FujinonSX800::ProtocolParser::describeFrame(frame, isTx);

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_frames.size() >= MaxFrames) {
        m_frames.pop_front();
    }
    m_frames.push_back(std::move(lf));
}

void TrafficView::clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_frames.clear();
}

void TrafficView::render(Canvas& canvas, int top, int bottom)
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

    canvas.drawBox(2, top, w - 4, availableHeight, boxStyle, true, false);
    canvas.drawString(4, top, " [7] PROTOCOL TRAFFIC INSPECTOR (PELCO-D PACKET SNIFFER) ", titleStyle);

    // Table Header
    Style hdrStyle {};
    hdrStyle.bg = Palette::CardBg;
    hdrStyle.fg = Palette::TextSecondary;
    hdrStyle.bold = true;
    canvas.fill(3, top + 1, w - 6, 1, " ", hdrStyle);

    canvas.drawString(4, top + 1, "TIMESTAMP", hdrStyle);
    canvas.drawString(18, top + 1, "DIR", hdrStyle);
    canvas.drawString(26, top + 1, "RAW HEX BYTES", hdrStyle);
    canvas.drawString(50, top + 1, "DECODED COMMAND / TELEMETRY NAME", hdrStyle);

    std::lock_guard<std::mutex> lock(m_mutex);
    const int visibleRows = availableHeight - 3;
    if (visibleRows <= 0) {
        return;
    }

    const int total = static_cast<int>(m_frames.size());
    const int startIdx = std::max(0, total - visibleRows);

    Style rowStyle {};
    rowStyle.bg = Palette::DarkBg;

    Style txStyle {};
    txStyle.bg = Palette::DarkBg;
    txStyle.fg = Palette::AccentBlue;
    txStyle.bold = true;

    Style rxStyle {};
    rxStyle.bg = Palette::DarkBg;
    rxStyle.fg = Palette::AccentGreen;
    rxStyle.bold = true;

    for (int r = 0; r < visibleRows && (startIdx + r) < total; ++r) {
        const auto& frame = m_frames[static_cast<size_t>(startIdx + r)];
        const int curY = top + 2 + r;

        rowStyle.fg = Palette::TextSecondary;
        canvas.drawString(4, curY, frame.timestamp, rowStyle);

        if (frame.isTx) {
            canvas.drawString(18, curY, std::string("TX ") + std::string(Symbols::ArrowFlowTx), txStyle);
        } else {
            canvas.drawString(18, curY, std::string("RX ") + std::string(Symbols::ArrowFlowRx), rxStyle);
        }

        rowStyle.fg = Palette::TextPrimary;
        canvas.drawString(26, curY, frame.hexStr, rowStyle);

        rowStyle.fg = frame.isTx ? Palette::AccentCyan : Palette::TextPrimary;
        canvas.drawString(50, curY, frame.description, rowStyle);
    }
}

} // namespace FujinonSX800Tui

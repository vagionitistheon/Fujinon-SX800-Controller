#include "Canvas.h"
#include "UtfSymbols.h"

#include <algorithm>

namespace FujinonSX800Tui {

namespace {

    size_t getUtf8CharLength(unsigned char lead) noexcept
    {
        if ((lead & 0x80U) == 0U) {
            return 1U;
        }
        if ((lead & 0xE0U) == 0xC0U) {
            return 2U;
        }
        if ((lead & 0xF0U) == 0xE0U) {
            return 3U;
        }
        if ((lead & 0xF8U) == 0xF0U) {
            return 4U;
        }
        return 1U;
    }

} // namespace

Canvas::Canvas(int width, int height)
{
    resize(width, height);
}

void Canvas::resize(int width, int height)
{
    m_width = std::max(10, width);
    m_height = std::max(5, height);

    const size_t totalCells = static_cast<size_t>(m_width * m_height);
    m_backBuffer.assign(totalCells, Cell {});
    m_frontBuffer.assign(totalCells, Cell { "", Style {} }); // Force full initial diff
}

void Canvas::clear(const Color& bg)
{
    Style s {};
    s.bg = bg;
    for (auto& cell : m_backBuffer) {
        cell.ch = " ";
        cell.style = s;
    }
}

void Canvas::setCell(int x, int y, std::string_view utfChar, const Style& style)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }
    Cell& c = m_backBuffer[index(x, y)];
    c.ch = utfChar;
    c.style = style;
}

void Canvas::drawString(int x, int y, std::string_view text, const Style& style)
{
    if (y < 0 || y >= m_height || x >= m_width) {
        return;
    }

    int curX = x;
    size_t i = 0U;
    while (i < text.size() && curX < m_width) {
        const size_t len = getUtf8CharLength(static_cast<unsigned char>(text[i]));
        const size_t actualLen = std::min(len, text.size() - i);
        const std::string_view utfCh = text.substr(i, actualLen);

        if (curX >= 0) {
            setCell(curX, y, utfCh, style);
        }
        curX++;
        i += actualLen;
    }
}

void Canvas::drawBox(int x, int y, int w, int h, const Style& style, bool rounded, bool doubleBorder)
{
    if (w < 2 || h < 2 || x >= m_width || y >= m_height) {
        return;
    }

    std::string_view cTL = doubleBorder ? Symbols::DblBoxTL : (rounded ? Symbols::RoundTL : Symbols::BoxTL);
    std::string_view cTR = doubleBorder ? Symbols::DblBoxTR : (rounded ? Symbols::RoundTR : Symbols::BoxTR);
    std::string_view cBL = doubleBorder ? Symbols::DblBoxBL : (rounded ? Symbols::RoundBL : Symbols::BoxBL);
    std::string_view cBR = doubleBorder ? Symbols::DblBoxBR : (rounded ? Symbols::RoundBR : Symbols::BoxBR);
    std::string_view cH = doubleBorder ? Symbols::DblBoxH : Symbols::BoxH;
    std::string_view cV = doubleBorder ? Symbols::DblBoxV : Symbols::BoxV;

    setCell(x, y, cTL, style);
    setCell(x + w - 1, y, cTR, style);
    setCell(x, y + h - 1, cBL, style);
    setCell(x + w - 1, y + h - 1, cBR, style);

    for (int col = x + 1; col < x + w - 1; ++col) {
        setCell(col, y, cH, style);
        setCell(col, y + h - 1, cH, style);
    }

    for (int row = y + 1; row < y + h - 1; ++row) {
        setCell(x, row, cV, style);
        setCell(x + w - 1, row, cV, style);
    }
}

void Canvas::fill(int x, int y, int w, int h, std::string_view ch, const Style& style)
{
    for (int r = y; r < y + h; ++r) {
        for (int c = x; c < x + w; ++c) {
            setCell(c, r, ch, style);
        }
    }
}

void Canvas::drawHorizontalBar(int x, int y, int w, double fraction, const Color& fillFg, const Color& fillBg,
    const Color& emptyFg, const Color& emptyBg, bool useSubBlocks)
{
    if (w <= 0 || y < 0 || y >= m_height) {
        return;
    }

    const double clamped = std::clamp(fraction, 0.0, 1.0);

    Style fillStyle {};
    fillStyle.fg = fillFg;
    fillStyle.bg = fillBg;

    Style emptyStyle {};
    emptyStyle.fg = emptyFg;
    emptyStyle.bg = emptyBg;

    if (!useSubBlocks) {
        const int filled = static_cast<int>(clamped * static_cast<double>(w));
        for (int i = 0; i < w; ++i) {
            if (i < filled) {
                setCell(x + i, y, Symbols::BlockFull, fillStyle);
            } else {
                setCell(x + i, y, Symbols::BlockLight, emptyStyle);
            }
        }
        return;
    }

    const int totalSubSteps = w * 8;
    const int filledSubSteps = static_cast<int>(clamped * static_cast<double>(totalSubSteps));
    const int fullBlocks = filledSubSteps / 8;
    const int remainder = filledSubSteps % 8;

    for (int i = 0; i < w; ++i) {
        if (i < fullBlocks) {
            setCell(x + i, y, Symbols::BlockFull, fillStyle);
        } else if (i == fullBlocks && remainder > 0) {
            Style partialStyle = fillStyle;
            partialStyle.bg = emptyBg;
            setCell(x + i, y, Symbols::SubBlocksH[remainder - 1], partialStyle);
        } else {
            setCell(x + i, y, " ", emptyStyle);
        }
    }
}

void Canvas::drawSlider(
    int x, int y, int w, double fraction, const Style& activeStyle, const Style& inactiveStyle, const Style& knobStyle)
{
    if (w <= 0 || y < 0 || y >= m_height) {
        return;
    }

    const double clamped = std::clamp(fraction, 0.0, 1.0);
    const int knobPos = std::clamp(static_cast<int>(clamped * static_cast<double>(w - 1)), 0, w - 1);

    for (int i = 0; i < w; ++i) {
        if (i < knobPos) {
            setCell(x + i, y, Symbols::BoxH, activeStyle);
        } else if (i == knobPos) {
            setCell(x + i, y, Symbols::DotActive, knobStyle);
        } else {
            setCell(x + i, y, Symbols::BoxH, inactiveStyle);
        }
    }
}

void Canvas::render(const Terminal& terminal)
{
    std::string out {};
    out.reserve(static_cast<size_t>(m_width * m_height * 8));

    int curX = -1;
    int curY = -1;
    Style curStyle {};
    bool styleInitialized = false;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const size_t idx = index(x, y);
            const Cell& back = m_backBuffer[idx];
            const Cell& front = m_frontBuffer[idx];

            if (back == front) {
                continue; // Cell unchanged
            }

            // Move cursor if not adjacent to current position
            if (curX != x || curY != y) {
                out += "\033[" + std::to_string(y + 1) + ";" + std::to_string(x + 1) + "H";
                curX = x;
                curY = y;
            }

            // Apply style delta
            if (!styleInitialized || curStyle != back.style) {
                out += "\033[0m"; // Reset attributes
                out += back.style.fg.toFgAnsi();
                out += back.style.bg.toBgAnsi();
                if (back.style.bold) {
                    out += "\033[1m";
                }
                if (back.style.dim) {
                    out += "\033[2m";
                }
                if (back.style.underline) {
                    out += "\033[4m";
                }

                curStyle = back.style;
                styleInitialized = true;
            }

            out += back.ch;
            curX++;
        }
    }

    if (styleInitialized) {
        out += "\033[0m";
    }

    if (!out.empty()) {
        terminal.writeRaw(out);
        terminal.flush();
    }

    m_frontBuffer = m_backBuffer;
}

} // namespace FujinonSX800Tui

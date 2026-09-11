#pragma once

/// @file Canvas.h
/// @brief Double-buffered 2D terminal canvas with ANSI delta rendering.
/// @author Theon Sensors / Maintainers

#include "Color.h"
#include "Terminal.h"

#include <string>
#include <string_view>
#include <vector>

namespace FujinonSX800Tui {

/// @struct Cell
/// @brief Single terminal cell supporting multi-byte UTF-8 graphemes and styling.
struct Cell {
    std::string ch { " " };
    Style style {};

    [[nodiscard]] bool operator==(const Cell& other) const noexcept
    {
        return style == other.style && ch == other.ch;
    }

    [[nodiscard]] bool operator!=(const Cell& other) const noexcept
    {
        return !(*this == other);
    }
};

/// @class Canvas
/// @brief High-performance double-buffered 2D screen buffer with delta ANSI emission.
class Canvas {
public:
    Canvas(int width = 80, int height = 24);
    ~Canvas() = default;

    void resize(int width, int height);
    [[nodiscard]] int width() const noexcept
    {
        return m_width;
    }
    [[nodiscard]] int height() const noexcept
    {
        return m_height;
    }

    void clear(const Color& bg = Palette::DarkBg);

    void setCell(int x, int y, std::string_view utfChar, const Style& style);
    void drawString(int x, int y, std::string_view text, const Style& style);
    void drawBox(int x, int y, int w, int h, const Style& style, bool rounded = true, bool doubleBorder = false);
    void fill(int x, int y, int w, int h, std::string_view ch, const Style& style);

    void drawHorizontalBar(int x, int y, int w, double fraction, const Color& fillFg, const Color& fillBg,
        const Color& emptyFg, const Color& emptyBg, bool useSubBlocks = true);

    void drawSlider(int x, int y, int w, double fraction, const Style& activeStyle, const Style& inactiveStyle,
        const Style& knobStyle);

    void render(const Terminal& terminal);

private:
    int m_width { 80 };
    int m_height { 24 };
    std::vector<Cell> m_frontBuffer {};
    std::vector<Cell> m_backBuffer {};

    [[nodiscard]] size_t index(int x, int y) const noexcept
    {
        return static_cast<size_t>(y * m_width + x);
    }
};

} // namespace FujinonSX800Tui

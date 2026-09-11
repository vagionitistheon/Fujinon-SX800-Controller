#pragma once

/// @file Color.h
/// @brief ANSI 24-bit TrueColor and styling definitions for the SX800 TUI.
/// @author Theon Sensors / Maintainers

#include <cstdint>
#include <string>

namespace FujinonSX800Tui {

/// @struct Color
/// @brief RGB representation with ANSI TrueColor escape sequence formatting.
struct Color {
    std::uint8_t r { 0U };
    std::uint8_t g { 0U };
    std::uint8_t b { 0U };
    bool isDefault { true };

    constexpr Color() noexcept = default;

    constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept
        : r(red)
        , g(green)
        , b(blue)
        , isDefault(false)
    {
    }

    [[nodiscard]] constexpr bool operator==(const Color& other) const noexcept
    {
        return isDefault == other.isDefault && r == other.r && g == other.g && b == other.b;
    }

    [[nodiscard]] constexpr bool operator!=(const Color& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] std::string toFgAnsi() const
    {
        if (isDefault) {
            return "\033[39m";
        }
        return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
    }

    [[nodiscard]] std::string toBgAnsi() const
    {
        if (isDefault) {
            return "\033[49m";
        }
        return "\033[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
    }
};

/// @struct Style
/// @brief Combined text styling and color attributes.
struct Style {
    Color fg {};
    Color bg {};
    bool bold { false };
    bool dim { false };
    bool underline { false };

    [[nodiscard]] constexpr bool operator==(const Style& other) const noexcept
    {
        return fg == other.fg && bg == other.bg && bold == other.bold && dim == other.dim
            && underline == other.underline;
    }

    [[nodiscard]] constexpr bool operator!=(const Style& other) const noexcept
    {
        return !(*this == other);
    }
};

namespace Palette {
    inline constexpr Color DarkBg { 13U, 17U, 23U }; // #0d1117
    inline constexpr Color PanelBg { 22U, 27U, 34U }; // #161b22
    inline constexpr Color CardBg { 33U, 38U, 45U }; // #21262d
    inline constexpr Color Border { 48U, 54U, 61U }; // #30363d
    inline constexpr Color BorderFocus { 88U, 166U, 255U }; // #58a6ff

    inline constexpr Color TextPrimary { 240U, 246U, 252U }; // #f0f6fc
    inline constexpr Color TextSecondary { 139U, 148U, 158U }; // #8b949e
    inline constexpr Color TextMuted { 72U, 79U, 88U }; // #484f58

    inline constexpr Color AccentBlue { 56U, 139U, 253U }; // #388bfd
    inline constexpr Color AccentCyan { 57U, 197U, 207U }; // #39c5cf
    inline constexpr Color AccentGreen { 63U, 185U, 80U }; // #3fb950
    inline constexpr Color AccentAmber { 210U, 153U, 34U }; // #d29922
    inline constexpr Color AccentRed { 248U, 81U, 73U }; // #f85149
    inline constexpr Color AccentPurple { 188U, 140U, 255U }; // #bc8cff
} // namespace Palette

} // namespace FujinonSX800Tui

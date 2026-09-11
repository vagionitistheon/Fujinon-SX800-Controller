#pragma once

/// @file UtfSymbols.h
/// @brief Curated UTF-8 box-drawing, block meter, and indicator glyphs.
/// @author Theon Sensors / Maintainers

#include <string_view>

namespace FujinonSX800Tui::Symbols {

// ---------- Single Line Box Drawing ----------
inline constexpr std::string_view BoxH = "─";
inline constexpr std::string_view BoxV = "│";
inline constexpr std::string_view BoxTL = "┌";
inline constexpr std::string_view BoxTR = "┐";
inline constexpr std::string_view BoxBL = "└";
inline constexpr std::string_view BoxBR = "┘";
inline constexpr std::string_view BoxTDown = "┬";
inline constexpr std::string_view BoxTUp = "┴";
inline constexpr std::string_view BoxTRight = "├";
inline constexpr std::string_view BoxTLeft = "┤";
inline constexpr std::string_view BoxCross = "┼";

// ---------- Rounded Corner Box Drawing ----------
inline constexpr std::string_view RoundTL = "╭";
inline constexpr std::string_view RoundTR = "╮";
inline constexpr std::string_view RoundBL = "╰";
inline constexpr std::string_view RoundBR = "╯";

// ---------- Double Line Box Drawing ----------
inline constexpr std::string_view DblBoxH = "═";
inline constexpr std::string_view DblBoxV = "║";
inline constexpr std::string_view DblBoxTL = "╔";
inline constexpr std::string_view DblBoxTR = "╗";
inline constexpr std::string_view DblBoxBL = "╚";
inline constexpr std::string_view DblBoxBR = "╝";
inline constexpr std::string_view DblBoxTDown = "╦";
inline constexpr std::string_view DblBoxTUp = "╩";
inline constexpr std::string_view DblBoxTRight = "╠";
inline constexpr std::string_view DblBoxTLeft = "╣";
inline constexpr std::string_view DblBoxCross = "╬";

// ---------- Shaded and Solid Blocks ----------
inline constexpr std::string_view BlockFull = "█";
inline constexpr std::string_view BlockDark = "▓";
inline constexpr std::string_view BlockMed = "▒";
inline constexpr std::string_view BlockLight = "░";

// ---------- Sub-Character Horizontal Fractional Blocks (1/8 to 8/8) ----------
inline constexpr std::string_view SubBlocksH[8] = { "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█" };

// ---------- Sub-Character Vertical Fractional Blocks (1/8 to 8/8) ----------
inline constexpr std::string_view SubBlocksV[8] = { " ", "▂", "▃", "▄", "▅", "▆", "▇", "█" };

// ---------- Indicator Glyphs & Badges ----------
inline constexpr std::string_view DotActive = "●";
inline constexpr std::string_view DotIdle = "○";
inline constexpr std::string_view DotPending = "◐";

inline constexpr std::string_view ArrowUp = "▲";
inline constexpr std::string_view ArrowDown = "▼";
inline constexpr std::string_view ArrowRight = "▶";
inline constexpr std::string_view ArrowLeft = "◀";

inline constexpr std::string_view ArrowFlowTx = "──▶";
inline constexpr std::string_view ArrowFlowRx = "◀──";

inline constexpr std::string_view IconConnected = "⚡";
inline constexpr std::string_view IconDisconnected = "🔌";
inline constexpr std::string_view IconTemp = "🌡";
inline constexpr std::string_view IconSun = "☀️";
inline constexpr std::string_view IconMoon = "🌙";
inline constexpr std::string_view IconOis = "🎯";
inline constexpr std::string_view IconClock = "🕒";
inline constexpr std::string_view IconCamera = "📷";
inline constexpr std::string_view IconSd = "💾";
inline constexpr std::string_view IconGear = "⚙";
inline constexpr std::string_view IconTraffic = "📡";
inline constexpr std::string_view IconCheck = "✔";
inline constexpr std::string_view IconCross = "✖";

} // namespace FujinonSX800Tui::Symbols

#pragma once

/// @file Terminal.h
/// @brief Cross-platform RAII terminal manager for raw mode and ANSI escape I/O.
/// @author Theon Sensors / Maintainers

#include <cstdint>
#include <optional>
#include <string_view>

#ifndef _WIN32
#include <termios.h>
#endif

namespace FujinonSX800Tui {

/// @enum Key
/// @brief Standardized keyboard and navigation keys.
enum class Key : std::uint16_t {
    None = 0,
    Char,
    Enter,
    Escape,
    Backspace,
    Tab,
    BackTab,
    Space,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12
};

/// @struct KeyEvent
/// @brief Decoded keystroke event with modifier flags.
struct KeyEvent {
    Key key { Key::None };
    char ch { '\0' };
    bool alt { false };
    bool ctrl { false };

    [[nodiscard]] constexpr bool isChar(char expected) const noexcept
    {
        return key == Key::Char && ch == expected;
    }
};

/// @class Terminal
/// @brief RAII console manager handling raw mode, alternate screen buffer, and keyboard input.
class Terminal {
public:
    Terminal();
    ~Terminal();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
    Terminal(Terminal&&) = delete;
    Terminal& operator=(Terminal&&) = delete;

    [[nodiscard]] bool initialize();
    void shutdown();

    [[nodiscard]] bool getSize(int& width, int& height) const;
    [[nodiscard]] bool checkResized();

    [[nodiscard]] std::optional<KeyEvent> pollKey();
    void writeRaw(std::string_view str) const;
    void flush() const;

private:
    bool m_initialized { false };

#ifndef _WIN32
    struct termios m_origTermios { };
#else
    unsigned long m_origInMode { 0 };
    unsigned long m_origOutMode { 0 };
#endif

    static void handleSigwinch(int sig);
};

} // namespace FujinonSX800Tui

#include "Terminal.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#ifndef _WIN32
#include <csignal>
#include <sys/ioctl.h>
#include <unistd.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace FujinonSX800Tui {

namespace {
    std::atomic<bool> g_resizedFlag { false };
}

Terminal::Terminal()
{
}

Terminal::~Terminal()
{
    shutdown();
}

void Terminal::handleSigwinch(int /*sig*/)
{
    g_resizedFlag.store(true, std::memory_order_relaxed);
}

bool Terminal::initialize()
{
    if (m_initialized) {
        return true;
    }

#ifndef _WIN32
    if (::tcgetattr(STDIN_FILENO, &m_origTermios) != 0) {
        return false;
    }

    struct termios raw = m_origTermios;
    raw.c_iflag &= static_cast<tcflag_t>(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
    raw.c_oflag &= static_cast<tcflag_t>(~(OPOST));
    raw.c_cflag |= static_cast<tcflag_t>(CS8);
    raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON | IEXTEN | ISIG));
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        return false;
    }

    struct sigaction sa { };
    sa.sa_handler = &Terminal::handleSigwinch;
    sa.sa_flags = SA_RESTART;
    ::sigaction(SIGWINCH, &sa, nullptr);
#else
    const HANDLE hIn = ::GetStdHandle(STD_INPUT_HANDLE);
    const HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    ::GetConsoleMode(hIn, &m_origInMode);
    ::GetConsoleMode(hOut, &m_origOutMode);

    const DWORD inMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    const DWORD outMode = m_origOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    ::SetConsoleMode(hIn, inMode);
    ::SetConsoleMode(hOut, outMode);
#endif

    m_initialized = true;

    // Switch to alternate screen buffer, clear screen, hide cursor
    writeRaw("\033[?1049h\033[2J\033[H\033[?25l");
    flush();

    return true;
}

void Terminal::shutdown()
{
    if (!m_initialized) {
        return;
    }

    // Show cursor, exit alternate screen buffer
    writeRaw("\033[?25h\033[?1049l");
    flush();

#ifndef _WIN32
    ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_origTermios);
#else
    const HANDLE hIn = ::GetStdHandle(STD_INPUT_HANDLE);
    const HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    ::SetConsoleMode(hIn, m_origInMode);
    ::SetConsoleMode(hOut, m_origOutMode);
#endif

    m_initialized = false;
}

bool Terminal::getSize(int& width, int& height) const
{
#ifndef _WIN32
    struct winsize ws { };
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        width = static_cast<int>(ws.ws_col);
        height = static_cast<int>(ws.ws_row);
        return true;
    }
#else
    CONSOLE_SCREEN_BUFFER_INFO csbi {};
    if (::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return true;
    }
#endif
    width = 80;
    height = 24;
    return false;
}

bool Terminal::checkResized()
{
    return g_resizedFlag.exchange(false, std::memory_order_relaxed);
}

void Terminal::writeRaw(std::string_view str) const
{
    if (!str.empty()) {
        std::fwrite(str.data(), 1, str.size(), stdout);
    }
}

void Terminal::flush() const
{
    std::fflush(stdout);
}

std::optional<KeyEvent> Terminal::pollKey()
{
#ifndef _WIN32
    char c { '\0' };
    const auto n = ::read(STDIN_FILENO, &c, 1);
    if (n <= 0) {
        return std::nullopt;
    }

    KeyEvent event {};

    if (c == '\033') {
        char seq[8] { '\0' };
        const auto n2 = ::read(STDIN_FILENO, &seq[0], 1);
        if (n2 <= 0) {
            event.key = Key::Escape;
            return event;
        }

        if (seq[0] == '[') {
            const auto n3 = ::read(STDIN_FILENO, &seq[1], 1);
            if (n3 <= 0) {
                return std::nullopt;
            }

            switch (seq[1]) {
            case 'A':
                event.key = Key::Up;
                return event;
            case 'B':
                event.key = Key::Down;
                return event;
            case 'C':
                event.key = Key::Right;
                return event;
            case 'D':
                event.key = Key::Left;
                return event;
            case 'H':
                event.key = Key::Home;
                return event;
            case 'F':
                event.key = Key::End;
                return event;
            case 'Z':
                event.key = Key::BackTab;
                return event; // Shift-Tab
            default:
                break;
            }

            if (seq[1] >= '0' && seq[1] <= '9') {
                const auto n4 = ::read(STDIN_FILENO, &seq[2], 1);
                if (n4 > 0 && seq[2] == '~') {
                    switch (seq[1]) {
                    case '1':
                        event.key = Key::Home;
                        return event;
                    case '4':
                        event.key = Key::End;
                        return event;
                    case '5':
                        event.key = Key::PageUp;
                        return event;
                    case '6':
                        event.key = Key::PageDown;
                        return event;
                    default:
                        break;
                    }
                }
            }
        } else if (seq[0] == 'O') {
            const auto n3 = ::read(STDIN_FILENO, &seq[1], 1);
            if (n3 > 0) {
                switch (seq[1]) {
                case 'P':
                    event.key = Key::F1;
                    return event;
                case 'Q':
                    event.key = Key::F2;
                    return event;
                case 'R':
                    event.key = Key::F3;
                    return event;
                case 'S':
                    event.key = Key::F4;
                    return event;
                default:
                    break;
                }
            }
        }

        return std::nullopt;
    }

    if (c == '\r' || c == '\n') {
        event.key = Key::Enter;
        return event;
    }
    if (c == '\t') {
        event.key = Key::Tab;
        return event;
    }
    if (c == 127 || c == '\b') {
        event.key = Key::Backspace;
        return event;
    }
    if (c == ' ') {
        event.key = Key::Space;
        return event;
    }
    if (static_cast<unsigned char>(c) >= 1 && static_cast<unsigned char>(c) <= 26) {
        event.key = Key::Char;
        event.ctrl = true;
        event.ch = static_cast<char>('a' + (c - 1));
        return event;
    }

    event.key = Key::Char;
    event.ch = c;
    return event;
#else
    const HANDLE hIn = ::GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || hIn == nullptr) {
        return std::nullopt;
    }

    DWORD numEvents = 0;
    while (::GetNumberOfConsoleInputEvents(hIn, &numEvents) != 0 && numEvents > 0) {
        INPUT_RECORD record {};
        DWORD eventsRead = 0;
        if (::ReadConsoleInputW(hIn, &record, 1, &eventsRead) == 0 || eventsRead == 0) {
            return std::nullopt;
        }

        if (record.EventType == WINDOW_BUFFER_SIZE_EVENT) {
            g_resizedFlag.store(true, std::memory_order_relaxed);
            continue;
        }

        if (record.EventType != KEY_EVENT) {
            continue;
        }

        const auto& ke = record.Event.KeyEvent;
        if (!ke.bKeyDown) {
            continue;
        }

        const WORD vk = ke.wVirtualKeyCode;

        // Ignore standalone modifier keystrokes
        if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT || vk == VK_CONTROL || vk == VK_LCONTROL
            || vk == VK_RCONTROL || vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU || vk == VK_CAPITAL
            || vk == VK_NUMLOCK || vk == VK_SCROLL) {
            continue;
        }

        const DWORD ctrlState = ke.dwControlKeyState;
        const bool alt = (ctrlState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
        const bool ctrl = (ctrlState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
        const bool shift = (ctrlState & SHIFT_PRESSED) != 0;

        KeyEvent event {};
        event.alt = alt;
        event.ctrl = ctrl;

        // Navigation and control keys
        switch (vk) {
        case VK_UP:
            event.key = Key::Up;
            return event;
        case VK_DOWN:
            event.key = Key::Down;
            return event;
        case VK_LEFT:
            event.key = Key::Left;
            return event;
        case VK_RIGHT:
            event.key = Key::Right;
            return event;
        case VK_HOME:
            event.key = Key::Home;
            return event;
        case VK_END:
            event.key = Key::End;
            return event;
        case VK_PRIOR:
            event.key = Key::PageUp;
            return event;
        case VK_NEXT:
            event.key = Key::PageDown;
            return event;
        case VK_RETURN:
            event.key = Key::Enter;
            return event;
        case VK_ESCAPE:
            event.key = Key::Escape;
            return event;
        case VK_BACK:
            event.key = Key::Backspace;
            return event;
        case VK_TAB:
            event.key = shift ? Key::BackTab : Key::Tab;
            return event;
        case VK_SPACE:
            event.key = Key::Space;
            event.ch = ' ';
            return event;
        default:
            break;
        }

        // Function keys F1 - F12
        if (vk >= VK_F1 && vk <= VK_F12) {
            event.key = static_cast<Key>(static_cast<std::uint16_t>(Key::F1) + (vk - VK_F1));
            return event;
        }

        // Ctrl + letter (e.g. Ctrl+C, Ctrl+Q)
        if (ctrl) {
            if (ke.uChar.AsciiChar >= 1 && ke.uChar.AsciiChar <= 26) {
                event.key = Key::Char;
                event.ctrl = true;
                event.ch = static_cast<char>('a' + (ke.uChar.AsciiChar - 1));
                return event;
            }
            if (vk >= 'A' && vk <= 'Z') {
                event.key = Key::Char;
                event.ctrl = true;
                event.ch = static_cast<char>('a' + (vk - 'A'));
                return event;
            }
        }

        // Standard Unicode/ASCII character input
        const wchar_t wch = ke.uChar.UnicodeChar;
        if (wch >= 32 && wch <= 126) {
            event.key = Key::Char;
            event.ch = static_cast<char>(wch);
            return event;
        }

        const char ach = ke.uChar.AsciiChar;
        if (ach >= 32 && ach <= 126) {
            event.key = Key::Char;
            event.ch = ach;
            return event;
        }

        // Fallback for alphanumeric keys with Alt or when char is not populated
        if (vk >= '0' && vk <= '9') {
            event.key = Key::Char;
            event.ch = static_cast<char>('0' + (vk - '0'));
            return event;
        }
        if (vk >= 'A' && vk <= 'Z') {
            event.key = Key::Char;
            event.ch = static_cast<char>((shift ? 'A' : 'a') + (vk - 'A'));
            return event;
        }
    }

    return std::nullopt;
#endif
}

} // namespace FujinonSX800Tui

#include "ConnectionModal.h"
#include "Color.h"
#include "UtfSymbols.h"

#include <algorithm>
#include <array>
#include <cstdlib>

namespace FujinonSX800Tui {

namespace {
    constexpr std::array<std::uint32_t, 6> kBaudRates { 2400U, 4800U, 9600U, 19200U, 38400U, 115200U };
}

ConnectionModal::ConnectionModal()
{
}

void ConnectionModal::open()
{
    m_isOpen = true;
    m_activeField = 0;
}

void ConnectionModal::close()
{
    m_isOpen = false;
}

bool ConnectionModal::isOpen() const noexcept
{
    return m_isOpen;
}

void ConnectionModal::setOnConnect(ConnectCallback cb)
{
    m_onConnect = std::move(cb);
}

void ConnectionModal::setDefaults(ConnectionType type, const std::string& info, std::uint8_t address)
{
    m_type = type;
    m_addressStr = std::to_string(address);

    if (type == ConnectionType::Serial && !info.empty()) {
        const auto atPos = info.find(" @");
        if (atPos != std::string::npos) {
            m_serialPort = info.substr(0, atPos);
        } else {
            m_serialPort = info;
        }
    } else if (type == ConnectionType::Tcp && !info.empty()) {
        const auto colonPos = info.find(':');
        if (colonPos != std::string::npos) {
            m_tcpHost = info.substr(0, colonPos);
            m_tcpPort = info.substr(colonPos + 1);
        }
    }
}

int ConnectionModal::maxFields() const noexcept
{
    switch (m_type) {
    case ConnectionType::Serial:
        return 6; // 0: Type, 1: Port, 2: Baud, 3: Address, 4: Connect, 5: Cancel
    case ConnectionType::Tcp:
        return 6; // 0: Type, 1: Host, 2: Port, 3: Address, 4: Connect, 5: Cancel
    case ConnectionType::Mock:
        return 4; // 0: Type, 1: Address, 2: Connect, 3: Cancel
    }
    return 4;
}

void ConnectionModal::submit()
{
    if (!m_onConnect) {
        close();
        return;
    }

    const auto addrVal = static_cast<std::uint8_t>(std::strtoul(m_addressStr.c_str(), nullptr, 10));
    const std::uint8_t address = (addrVal == 0U) ? 7U : addrVal;

    if (m_type == ConnectionType::Serial) {
        const std::uint32_t baud = kBaudRates[std::min(m_baudIndex, kBaudRates.size() - 1)];
        m_onConnect(m_type, m_serialPort, baud, address);
    } else if (m_type == ConnectionType::Tcp) {
        const auto portVal = static_cast<std::uint32_t>(std::strtoul(m_tcpPort.c_str(), nullptr, 10));
        m_onConnect(m_type, m_tcpHost, (portVal == 0U) ? 5000U : portVal, address);
    } else {
        m_onConnect(m_type, "Mock Device", 0U, address);
    }

    close();
}

void ConnectionModal::handleInput(const KeyEvent& ev)
{
    if (!m_isOpen) {
        return;
    }

    if (ev.key == Key::Escape) {
        close();
        return;
    }

    const int total = maxFields();

    // Direct Tab switching with 1, 2, 3
    if (ev.key == Key::Char && ev.ch == '1') {
        m_type = ConnectionType::Serial;
        m_activeField = std::min(m_activeField, maxFields() - 1);
        return;
    }
    if (ev.key == Key::Char && ev.ch == '2') {
        m_type = ConnectionType::Tcp;
        m_activeField = std::min(m_activeField, maxFields() - 1);
        return;
    }
    if (ev.key == Key::Char && ev.ch == '3') {
        m_type = ConnectionType::Mock;
        m_activeField = std::min(m_activeField, maxFields() - 1);
        return;
    }

    // Navigation between fields
    if (ev.key == Key::Tab || ev.key == Key::Down) {
        m_activeField = (m_activeField + 1) % total;
        return;
    }
    if (ev.key == Key::BackTab || ev.key == Key::Up) {
        m_activeField = (m_activeField + total - 1) % total;
        return;
    }

    // Connect / Submit
    if (ev.key == Key::Enter) {
        const int cancelField = total - 1;
        if (m_activeField == cancelField) {
            close();
        } else {
            submit();
        }
        return;
    }

    // Left / Right adjustments
    if (m_activeField == 0) {
        // Switch connection type
        int currentType = static_cast<int>(m_type);
        if (ev.key == Key::Left) {
            currentType = (currentType + 2) % 3;
            m_type = static_cast<ConnectionType>(currentType);
            m_activeField = 0;
            return;
        }
        if (ev.key == Key::Right || ev.key == Key::Space) {
            currentType = (currentType + 1) % 3;
            m_type = static_cast<ConnectionType>(currentType);
            m_activeField = 0;
            return;
        }
    } else if (m_type == ConnectionType::Serial && m_activeField == 2) {
        // Baud rate selector
        if (ev.key == Key::Left) {
            m_baudIndex = (m_baudIndex + kBaudRates.size() - 1) % kBaudRates.size();
            return;
        }
        if (ev.key == Key::Right || ev.key == Key::Space) {
            m_baudIndex = (m_baudIndex + 1) % kBaudRates.size();
            return;
        }
    }

    // Text editing on editable fields
    auto editString = [&](std::string& target) {
        if (ev.key == Key::Backspace) {
            if (!target.empty()) {
                target.pop_back();
            }
        } else if (ev.key == Key::Char && !ev.ctrl && !ev.alt) {
            if (target.size() < 40 && ev.ch >= 32 && ev.ch <= 126) {
                target.push_back(ev.ch);
            }
        }
    };

    if (m_type == ConnectionType::Serial) {
        if (m_activeField == 1) {
            editString(m_serialPort);
        } else if (m_activeField == 3) {
            editString(m_addressStr);
        }
    } else if (m_type == ConnectionType::Tcp) {
        if (m_activeField == 1) {
            editString(m_tcpHost);
        } else if (m_activeField == 2) {
            editString(m_tcpPort);
        } else if (m_activeField == 3) {
            editString(m_addressStr);
        }
    } else if (m_type == ConnectionType::Mock) {
        if (m_activeField == 1) {
            editString(m_addressStr);
        }
    }
}

void ConnectionModal::render(Canvas& canvas)
{
    if (!m_isOpen) {
        return;
    }

    const int w = canvas.width();
    const int h = canvas.height();

    const int modalW = 62;
    const int modalH = 16;
    const int modalX = std::max(1, (w - modalW) / 2);
    const int modalY = std::max(1, (h - modalH) / 2);

    // Dim background shadow / fill modal area
    Style modalBg {};
    modalBg.bg = Palette::PanelBg;
    modalBg.fg = Palette::TextPrimary;
    canvas.fill(modalX, modalY, modalW, modalH, " ", modalBg);

    // Outer double border
    Style borderStyle {};
    borderStyle.bg = Palette::PanelBg;
    borderStyle.fg = Palette::AccentCyan;
    borderStyle.bold = true;
    canvas.drawBox(modalX, modalY, modalW, modalH, borderStyle, false, true);

    // Title banner
    Style titleStyle {};
    titleStyle.bg = Palette::PanelBg;
    titleStyle.fg = Palette::AccentCyan;
    titleStyle.bold = true;
    canvas.drawString(modalX + 16, modalY, " [ CONNECTION MANAGER ] ", titleStyle);

    // Type Tabs (Field 0)
    const int tabRow = modalY + 2;
    canvas.drawString(modalX + 4, tabRow, "Type: ", modalBg);

    auto drawTypePill = [&](int x, ConnectionType type, const char* label, bool focused) {
        Style pillStyle {};
        pillStyle.bg = (m_type == type) ? Palette::AccentBlue : Palette::DarkBg;
        pillStyle.fg = (m_type == type) ? Palette::DarkBg : Palette::TextPrimary;
        pillStyle.bold = true;
        if (focused) {
            pillStyle.underline = true;
        }
        canvas.drawString(x, tabRow, label, pillStyle);
    };

    const bool typeFocused = (m_activeField == 0);
    drawTypePill(modalX + 11, ConnectionType::Serial, " [1] Serial ", typeFocused && m_type == ConnectionType::Serial);
    drawTypePill(modalX + 25, ConnectionType::Tcp, " [2] TCP/IP ", typeFocused && m_type == ConnectionType::Tcp);
    drawTypePill(modalX + 39, ConnectionType::Mock, " [3] Mock ", typeFocused && m_type == ConnectionType::Mock);

    // Divider
    Style divStyle {};
    divStyle.bg = Palette::PanelBg;
    divStyle.fg = Palette::Border;
    canvas.drawString(modalX + 2, modalY + 3, "──────────────────────────────────────────────────────────", divStyle);

    auto drawField = [&](int row, const char* label, const std::string& value, bool focused) {
        Style lblStyle {};
        lblStyle.bg = Palette::PanelBg;
        lblStyle.fg = focused ? Palette::AccentCyan : Palette::TextSecondary;
        lblStyle.bold = focused;
        canvas.drawString(modalX + 4, row, label, lblStyle);

        Style inputStyle {};
        inputStyle.bg = focused ? Palette::DarkBg : Palette::CardBg;
        inputStyle.fg = focused ? Palette::AccentAmber : Palette::TextPrimary;
        inputStyle.bold = focused;

        const int fieldWidth = 32;
        canvas.fill(modalX + 24, row, fieldWidth, 1, " ", inputStyle);
        std::string displayVal = value;
        if (focused) {
            displayVal += "_"; // Blinking-style cursor
        }
        canvas.drawString(modalX + 25, row, displayVal, inputStyle);
    };

    if (m_type == ConnectionType::Serial) {
        drawField(modalY + 5, "Device Path:   ", m_serialPort, m_activeField == 1);

        const std::string baudStr = "< " + std::to_string(kBaudRates[m_baudIndex]) + " bps >";
        drawField(modalY + 7, "Baud Rate:     ", baudStr, m_activeField == 2);

        drawField(modalY + 9, "Camera RS-485: ", m_addressStr, m_activeField == 3);
    } else if (m_type == ConnectionType::Tcp) {
        drawField(modalY + 5, "Target Host/IP:", m_tcpHost, m_activeField == 1);
        drawField(modalY + 7, "TCP Port:      ", m_tcpPort, m_activeField == 2);
        drawField(modalY + 9, "Camera RS-485: ", m_addressStr, m_activeField == 3);
    } else {
        drawField(modalY + 5, "Camera RS-485: ", m_addressStr, m_activeField == 1);

        Style infoStyle {};
        infoStyle.bg = Palette::PanelBg;
        infoStyle.fg = Palette::TextMuted;
        canvas.drawString(modalX + 6, modalY + 7, "● Simulates virtual Fujinon SX800 hardware", infoStyle);
        canvas.drawString(modalX + 6, modalY + 8, "● Generates telemetry, ACK/NAK & lens motion", infoStyle);
    }

    // Action Buttons
    const int total = maxFields();
    const int connectIdx = total - 2;
    const int cancelIdx = total - 1;

    const int btnRow = modalY + 12;

    Style connBtnStyle {};
    connBtnStyle.bg = (m_activeField == connectIdx) ? Palette::AccentGreen : Palette::DarkBg;
    connBtnStyle.fg = (m_activeField == connectIdx) ? Palette::DarkBg : Palette::AccentGreen;
    connBtnStyle.bold = true;
    canvas.drawString(modalX + 16, btnRow, "  [ CONNECT ]  ", connBtnStyle);

    Style cancelBtnStyle {};
    cancelBtnStyle.bg = (m_activeField == cancelIdx) ? Palette::AccentRed : Palette::DarkBg;
    cancelBtnStyle.fg = (m_activeField == cancelIdx) ? Palette::DarkBg : Palette::AccentRed;
    cancelBtnStyle.bold = true;
    canvas.drawString(modalX + 34, btnRow, "  [ CANCEL ]  ", cancelBtnStyle);

    // Modal Footer Hint
    Style hintStyle {};
    hintStyle.bg = Palette::PanelBg;
    hintStyle.fg = Palette::TextMuted;
    canvas.drawString(modalX + 5, modalY + 14, "[Tab/Arrows] Navigate │ [Enter] Connect │ [Esc] Cancel", hintStyle);
}

} // namespace FujinonSX800Tui

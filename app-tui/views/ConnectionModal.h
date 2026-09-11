#pragma once

/// @file ConnectionModal.h
/// @brief Interactive connection configuration modal dialog for the TUI client.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "Terminal.h"

#include <cstdint>
#include <functional>
#include <string>

namespace FujinonSX800Tui {

/// @enum ConnectionType
/// @brief Target communication transport type.
enum class ConnectionType : std::uint8_t { Serial = 0, Tcp, Mock };

/// @class ConnectionModal
/// @brief Interactive popup modal allowing runtime switching between Serial, TCP, and Mock connections.
class ConnectionModal {
public:
    ConnectionModal();
    ~ConnectionModal() = default;

    ConnectionModal(const ConnectionModal&) = delete;
    ConnectionModal& operator=(const ConnectionModal&) = delete;
    ConnectionModal(ConnectionModal&&) = default;
    ConnectionModal& operator=(ConnectionModal&&) = default;

    void open();
    void close();
    [[nodiscard]] bool isOpen() const noexcept;

    void handleInput(const KeyEvent& ev);
    void render(Canvas& canvas);

    using ConnectCallback = std::function<void(
        ConnectionType type, const std::string& endpoint, std::uint32_t baudOrPort, std::uint8_t address)>;
    void setOnConnect(ConnectCallback cb);

    void setDefaults(ConnectionType type, const std::string& info, std::uint8_t address);

private:
    void submit();
    [[nodiscard]] int maxFields() const noexcept;

    bool m_isOpen { false };
    ConnectionType m_type { ConnectionType::Serial };
    int m_activeField { 0 };

    std::string m_serialPort { "/dev/ttyUSB0" };
    std::size_t m_baudIndex { 2 }; // 9600
    std::string m_tcpHost { "127.0.0.1" };
    std::string m_tcpPort { "5000" };
    std::string m_addressStr { "7" };

    ConnectCallback m_onConnect {};
};

} // namespace FujinonSX800Tui

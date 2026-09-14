#pragma once

/// @file UdpTransport.h
/// @brief Cross-platform UDP transport for Pelco-D over IP bridges.

#include "ITransport.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

namespace FujinonSX800 {

/// @class UdpTransport
/// @brief Connected UDP datagram transport with optional local-port binding.
/// @note Connected means the local socket is configured; UDP does not confirm
///       that a remote camera is reachable.
class UdpTransport final : public ITransport {
public:
    explicit UdpTransport(std::string host = "192.168.1.100", std::uint16_t port = 4001U, std::uint16_t localPort = 0U);
    ~UdpTransport() override;

    UdpTransport(const UdpTransport&) = delete;
    UdpTransport& operator=(const UdpTransport&) = delete;
    UdpTransport(UdpTransport&&) = delete;
    UdpTransport& operator=(UdpTransport&&) = delete;

    void setHost(const std::string& host);
    [[nodiscard]] std::string getHost() const;
    void setPort(std::uint16_t port);
    [[nodiscard]] std::uint16_t getPort() const noexcept;
    void setLocalPort(std::uint16_t localPort);
    [[nodiscard]] std::uint16_t getLocalPort() const noexcept;

    [[nodiscard]] bool open() override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] bool sendData(const std::vector<std::uint8_t>& data) override;
    void setDataCallback(DataReceivedCallback callback) override;
    void setStateCallback(StateChangedCallback callback) override;

private:
    void readWorker();
    void notifyState(TransportState state, const std::string& message);

    std::string m_host;
    std::uint16_t m_port { 4001U };
    std::uint16_t m_localPort { 0U };

#ifdef _WIN32
    using SocketHandle = std::uintptr_t;
    static constexpr SocketHandle InvalidSocket { ~static_cast<SocketHandle>(0) };
#else
    using SocketHandle = int;
    static constexpr SocketHandle InvalidSocket { -1 };
#endif

    std::atomic<SocketHandle> m_socket { InvalidSocket };
    std::atomic<bool> m_running { false };
    std::thread m_readThread;

    mutable std::mutex m_callbackMutex;
    DataReceivedCallback m_dataCallback;
    StateChangedCallback m_stateCallback;
    mutable std::mutex m_configMutex;
    mutable std::mutex m_writeMutex;
};

} // namespace FujinonSX800

/// @file UdpTransport.cpp
/// @brief Implementation of the cross-platform UDP transport.

#include "UdpTransport.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define CLOSE_SOCKET(s) ::closesocket(s)
#define POLL_SOCKET(fds, nfds, timeout) ::WSAPoll(fds, nfds, timeout)
#define IS_WOULDBLOCK() (::WSAGetLastError() == WSAEWOULDBLOCK)
#else
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define CLOSE_SOCKET(s) ::close(s)
#define POLL_SOCKET(fds, nfds, timeout) ::poll(fds, nfds, timeout)
#define IS_WOULDBLOCK() (errno == EAGAIN || errno == EWOULDBLOCK)
#endif

#include <array>
#include <glog/logging.h>

namespace FujinonSX800 {

namespace {

#ifdef _WIN32
    struct WinsockInit {
        WinsockInit()
        {
            WSADATA data {};
            ::WSAStartup(MAKEWORD(2, 2), &data);
        }

        ~WinsockInit()
        {
            ::WSACleanup();
        }
    };

    void ensureWinsockInitialized()
    {
        static WinsockInit init {};
        static_cast<void>(init);
    }

    std::string socketError()
    {
        return "Winsock error " + std::to_string(::WSAGetLastError());
    }
#else
    std::string socketError()
    {
        return std::strerror(errno);
    }
#endif

#ifdef _WIN32
    bool setNonBlocking(SOCKET socket, bool enabled)
    {
        u_long mode = enabled ? 1UL : 0UL;
        return ::ioctlsocket(socket, FIONBIO, &mode) == 0;
#else
    bool setNonBlocking(int socket, bool enabled)
    {
        const int flags = ::fcntl(socket, F_GETFL, 0);
        if (flags < 0) {
            return false;
        }
        const int updated = enabled ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        return ::fcntl(socket, F_SETFL, updated) == 0;
    }
#endif

    } // namespace

    UdpTransport::UdpTransport(std::string host, std::uint16_t port, std::uint16_t localPort)
        : m_host { std::move(host) }
        , m_port { port }
        , m_localPort { localPort }
    {
#ifdef _WIN32
        ensureWinsockInitialized();
#endif
    }

    UdpTransport::~UdpTransport()
    {
        close();
    }

    void UdpTransport::setHost(const std::string& host)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        if (!isOpen()) {
            m_host = host;
        }
    }

    std::string UdpTransport::getHost() const
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        return m_host;
    }

    void UdpTransport::setPort(std::uint16_t port)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        if (!isOpen()) {
            m_port = port;
        }
    }

    std::uint16_t UdpTransport::getPort() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        return m_port;
    }

    void UdpTransport::setLocalPort(std::uint16_t localPort)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        if (!isOpen()) {
            m_localPort = localPort;
        }
    }

    std::uint16_t UdpTransport::getLocalPort() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        return m_localPort;
    }

    bool UdpTransport::isOpen() const noexcept
    {
        return m_running.load() && m_socket.load() != InvalidSocket;
    }

    bool UdpTransport::open()
    {
        close();

        std::string host;
        std::uint16_t port { 0U };
        std::uint16_t localPort { 0U };
        {
            std::lock_guard<std::mutex> lock(m_configMutex);
            host = m_host;
            port = m_port;
            localPort = m_localPort;
        }

        if (host.empty() || port == 0U) {
            notifyState(TransportState::Error, host.empty() ? "UDP host is empty" : "UDP port is zero");
            return false;
        }

#ifdef _WIN32
        ensureWinsockInitialized();
#endif
        notifyState(TransportState::Connecting, "Connecting to " + host + ":" + std::to_string(port));

        struct addrinfo hints { };
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        struct addrinfo* result { nullptr };
        const std::string portText = std::to_string(port);
        const int resolveResult = ::getaddrinfo(host.c_str(), portText.c_str(), &hints, &result);
        if (resolveResult != 0 || result == nullptr) {
            notifyState(TransportState::Error, "UDP host resolution failed for " + host);
            return false;
        }

        SocketHandle selected { InvalidSocket };
        for (const struct addrinfo* entry = result; entry != nullptr; entry = entry->ai_next) {
            const auto socket
                = static_cast<SocketHandle>(::socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol));
            if (socket == InvalidSocket) {
                continue;
            }

            bool configured = true;
            if (localPort != 0U) {
                if (entry->ai_family == AF_INET) {
                    struct sockaddr_in localAddress { };
                    localAddress.sin_family = AF_INET;
                    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
                    localAddress.sin_port = htons(localPort);
                    configured
                        = ::bind(socket, reinterpret_cast<const sockaddr*>(&localAddress), sizeof(localAddress)) == 0;
                } else if (entry->ai_family == AF_INET6) {
                    struct sockaddr_in6 localAddress { };
                    localAddress.sin6_family = AF_INET6;
                    localAddress.sin6_addr = in6addr_any;
                    localAddress.sin6_port = htons(localPort);
                    configured
                        = ::bind(socket, reinterpret_cast<const sockaddr*>(&localAddress), sizeof(localAddress)) == 0;
                } else {
                    configured = false;
                }
            }

            if (configured) {
#ifdef _WIN32
                configured = ::connect(socket, entry->ai_addr, static_cast<int>(entry->ai_addrlen)) == 0;
#else
            configured = ::connect(socket, entry->ai_addr, static_cast<socklen_t>(entry->ai_addrlen)) == 0;
#endif
            }
            if (configured) {
                configured = setNonBlocking(socket, true);
            }
            if (configured) {
                selected = socket;
                break;
            }
            CLOSE_SOCKET(socket);
        }
        ::freeaddrinfo(result);

        if (selected == InvalidSocket) {
            notifyState(TransportState::Error, "Failed to create UDP socket to " + host + ":" + portText);
            return false;
        }

        m_socket = selected;
        m_running = true;
        m_readThread = std::thread(&UdpTransport::readWorker, this);
        notifyState(TransportState::Connected, "Connected to " + host + ":" + portText);
        return true;
    }

    void UdpTransport::close()
    {
        const bool wasRunning = m_running.exchange(false);
        const auto socket = m_socket.exchange(InvalidSocket);
        if (socket != InvalidSocket) {
            CLOSE_SOCKET(socket);
        }
        if (m_readThread.joinable()) {
            m_readThread.join();
        }
        if (wasRunning || socket != InvalidSocket) {
            notifyState(TransportState::Disconnected, "UDP socket closed");
        }
    }

    bool UdpTransport::sendData(const std::vector<std::uint8_t>& data)
    {
        if (!isOpen() || data.empty()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_writeMutex);
        const auto socket = m_socket.load();
        if (socket == InvalidSocket) {
            return false;
        }

#ifdef _WIN32
        const int sent = ::send(socket, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()), 0);
        const bool complete = sent == static_cast<int>(data.size());
#else
    const ssize_t sent = ::send(socket, data.data(), data.size(), MSG_NOSIGNAL);
    const bool complete = sent == static_cast<ssize_t>(data.size());
#endif
        if (!complete) {
            notifyState(TransportState::Error, "UDP send failed: " + socketError());
        }
        return complete;
    }

    void UdpTransport::setDataCallback(DataReceivedCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_dataCallback = std::move(callback);
    }

    void UdpTransport::setStateCallback(StateChangedCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_stateCallback = std::move(callback);
    }

    void UdpTransport::readWorker()
    {
        std::array<std::uint8_t, 2048> buffer {};
        while (m_running.load()) {
            const auto socket = m_socket.load();
            if (socket == InvalidSocket) {
                break;
            }

#ifdef _WIN32
            WSAPOLLFD pollFd {};
            pollFd.fd = socket;
#else
        struct pollfd pollFd { };
        pollFd.fd = socket;
#endif
            pollFd.events = POLLIN;
            const int pollResult = POLL_SOCKET(&pollFd, 1, 100);
            if (!m_running.load()) {
                break;
            }
            if (pollResult < 0) {
                if (!m_running.load()) {
                    break;
                }
                notifyState(TransportState::Error, "UDP poll failed: " + socketError());
                break;
            }
            if (pollResult == 0 || (pollFd.revents & POLLIN) == 0) {
                continue;
            }

#ifdef _WIN32
            const int received
                = ::recv(socket, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
#else
        const ssize_t received = ::recv(socket, buffer.data(), buffer.size(), 0);
#endif
            if (received > 0) {
                DataReceivedCallback callback;
                {
                    std::lock_guard<std::mutex> lock(m_callbackMutex);
                    callback = m_dataCallback;
                }
                if (callback) {
                    callback(buffer.data(), static_cast<std::size_t>(received));
                }
            } else if (received < 0 && !IS_WOULDBLOCK() && m_running.load()) {
                notifyState(TransportState::Error, "UDP receive failed: " + socketError());
                break;
            }
        }
    }

    void UdpTransport::notifyState(TransportState state, const std::string& message)
    {
        StateChangedCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callback = m_stateCallback;
        }
        if (callback) {
            callback(state, message);
        }
    }

} // namespace FujinonSX800

#include "TcpTransport.h"

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
#define SEND_FLAGS 0
#define IS_WOULDBLOCK() (::WSAGetLastError() == WSAEWOULDBLOCK)
#else
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define CLOSE_SOCKET(s) ::close(s)
#define POLL_SOCKET(fds, nfds, timeout) ::poll(fds, nfds, timeout)
#define SEND_FLAGS MSG_NOSIGNAL
#define IS_WOULDBLOCK() (errno == EAGAIN || errno == EWOULDBLOCK)
#endif

#include <array>
#include <chrono>
#include <glog/logging.h>

namespace FujinonSX800 {

namespace {

#ifdef _WIN32
    struct WinsockInit {
        WinsockInit()
        {
            WSADATA wsaData {};
            ::WSAStartup(MAKEWORD(2, 2), &wsaData);
        }
        ~WinsockInit()
        {
            ::WSACleanup();
        }
    };

    void ensureWinsockInitialized()
    {
        static WinsockInit init;
    }

    std::string getSocketErrorString(int errCode = 0)
    {
        if (errCode == 0) {
            errCode = ::WSAGetLastError();
        }
        char* errText = nullptr;
        const DWORD len = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
            errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&errText), 0, nullptr);
        std::string msg = (len > 0 && errText) ? std::string(errText) : "Winsock error " + std::to_string(errCode);
        if (errText) {
            LocalFree(errText);
        }
        while (!msg.empty() && (msg.back() == '\r' || msg.back() == '\n')) {
            msg.pop_back();
        }
        return msg;
    }

    bool setNonBlocking(SOCKET s, bool nonBlocking)
    {
        u_long mode = nonBlocking ? 1 : 0;
        return ::ioctlsocket(s, FIONBIO, &mode) == 0;
    }

#else

    std::string getSocketErrorString(int errCode = 0)
    {
        if (errCode == 0) {
            errCode = errno;
        }
        return std::string(std::strerror(errCode));
    }

    bool setNonBlocking(int fd, bool nonBlocking)
    {
        const int flags = ::fcntl(fd, F_GETFL, 0);
        if (flags == -1) {
            return false;
        }
        const int newFlags = nonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        return ::fcntl(fd, F_SETFL, newFlags) == 0;
    }
#endif

} // namespace

TcpTransport::TcpTransport(std::string host, std::uint16_t port)
    : m_host { std::move(host) }
    , m_port { port }
{
#ifdef _WIN32
    ensureWinsockInitialized();
#endif
}

TcpTransport::~TcpTransport()
{
    close();
}

void TcpTransport::setHost(const std::string& host)
{
    std::lock_guard<std::mutex> lock(m_writeMutex);
    m_host = host;
}

std::string TcpTransport::getHost() const
{
    std::lock_guard<std::mutex> lock(m_writeMutex);
    return m_host;
}

void TcpTransport::setPort(std::uint16_t port)
{
    std::lock_guard<std::mutex> lock(m_writeMutex);
    m_port = port;
}

std::uint16_t TcpTransport::getPort() const noexcept
{
    return m_port;
}

bool TcpTransport::open()
{
    close();

    std::string host;
    std::uint16_t port { 4001U };
    {
        std::lock_guard<std::mutex> lock(m_writeMutex);
        host = m_host;
        port = m_port;
    }

    if (host.empty()) {
        LOG(ERROR) << "Cannot open TCP transport: host address is empty";
        notifyState(TransportState::Error, "Host address is empty");
        return false;
    }

    LOG(INFO) << "Connecting to TCP host " << host << ":" << port;
    notifyState(TransportState::Connecting, "Connecting to " + host + ":" + std::to_string(port));

    struct addrinfo hints { };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* res = nullptr;
    const std::string portStr = std::to_string(port);
    const int gaiErr = ::getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (gaiErr != 0 || res == nullptr) {
        LOG(ERROR) << "DNS resolution failed for " << host << ": " << ::gai_strerror(gaiErr);
        notifyState(TransportState::Error, "DNS resolution failed for " + host);
        return false;
    }

    SocketHandle connectedSock = InvalidSocket;
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        const auto s = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#ifdef _WIN32
        if (s == INVALID_SOCKET) {
            continue;
        }
#else
        if (s < 0) {
            continue;
        }
#endif

        int flag = 1;
        ::setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&flag), sizeof(flag));

        // Non-blocking connection with 2-second timeout
        setNonBlocking(s, true);

#ifdef _WIN32
        const int connResult = ::connect(s, p->ai_addr, static_cast<int>(p->ai_addrlen));
#else
        const int connResult = ::connect(s, p->ai_addr, static_cast<socklen_t>(p->ai_addrlen));
#endif
        bool connectedOk = (connResult == 0);

        if (!connectedOk) {
#ifdef _WIN32
            const int err = ::WSAGetLastError();
            const bool inProgress = (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS);
#else
            const bool inProgress = (errno == EINPROGRESS);
#endif

            if (inProgress) {
                fd_set writeFds;
                FD_ZERO(&writeFds);
                FD_SET(s, &writeFds);

                struct timeval timeout { };
                timeout.tv_sec = 2; // 2 seconds connect timeout
                timeout.tv_usec = 0;

                const int selRes = ::select(static_cast<int>(s + 1), nullptr, &writeFds, nullptr, &timeout);
                if (selRes > 0) {
                    int sockErr = 0;
#ifdef _WIN32
                    int len = sizeof(sockErr);
#else
                    socklen_t len = sizeof(sockErr);
#endif
                    ::getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&sockErr), &len);
                    if (sockErr == 0) {
                        connectedOk = true;
                    }
                }
            }
        }

        if (connectedOk) {
            setNonBlocking(s, false);
            connectedSock = static_cast<SocketHandle>(s);
            break;
        }

        CLOSE_SOCKET(s);
    }

    ::freeaddrinfo(res);

    if (connectedSock == InvalidSocket) {
        LOG(ERROR) << "Failed to establish TCP connection to " << host << ":" << portStr;
        notifyState(TransportState::Error, "Failed to connect to " + host + ":" + portStr);
        return false;
    }

    m_sockfd = connectedSock;
    m_running = true;
    m_readThread = std::thread(&TcpTransport::readWorker, this);

    LOG(INFO) << "TCP connection established to " << host << ":" << portStr;
    notifyState(TransportState::Connected, "Connected to " + host + ":" + portStr);
    return true;
}

void TcpTransport::close()
{
    m_running = false;

    if (m_sockfd != InvalidSocket) {
#ifdef _WIN32
        ::shutdown(static_cast<SOCKET>(m_sockfd), SD_BOTH);
#else
        ::shutdown(m_sockfd, SHUT_RDWR);
#endif
    }

    if (m_readThread.joinable()) {
        m_readThread.join();
    }

    if (m_sockfd != InvalidSocket) {
        LOG(INFO) << "Closing TCP socket";
        CLOSE_SOCKET(m_sockfd);
        m_sockfd = InvalidSocket;
        notifyState(TransportState::Disconnected, "Socket closed");
    }
}

bool TcpTransport::isOpen() const noexcept
{
    return m_sockfd != InvalidSocket && m_running.load();
}

bool TcpTransport::sendData(const std::vector<std::uint8_t>& data)
{
    if (!isOpen() || data.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_writeMutex);
    std::size_t totalSent { 0U };
    const std::size_t toSend { data.size() };

    while (totalSent < toSend && m_running.load()) {
        const auto sent = ::send(
#ifdef _WIN32
            static_cast<SOCKET>(m_sockfd), reinterpret_cast<const char*>(data.data() + totalSent),
            static_cast<int>(toSend - totalSent), SEND_FLAGS
#else
            m_sockfd, data.data() + totalSent, toSend - totalSent, SEND_FLAGS
#endif
        );

        if (sent > 0) {
            totalSent += static_cast<std::size_t>(sent);
        } else if (sent < 0) {
            if (IS_WOULDBLOCK()) {
#ifdef _WIN32
                WSAPOLLFD pfd {};
                pfd.fd = static_cast<SOCKET>(m_sockfd);
                pfd.events = POLLOUT;
                POLL_SOCKET(&pfd, 1, 50);
#else
                struct pollfd pfd { };
                pfd.fd = m_sockfd;
                pfd.events = POLLOUT;
                POLL_SOCKET(&pfd, 1, 50);
#endif
                continue;
            }
            return false;
        }
    }

    if (totalSent == toSend) {
        VLOG(1) << "TCP TX: " << totalSent << " bytes";
        return true;
    }
    LOG(WARNING) << "TCP TX incomplete: sent " << totalSent << " of " << toSend << " bytes";
    return false;
}

void TcpTransport::setDataCallback(DataReceivedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_dataCallback = std::move(callback);
}

void TcpTransport::setStateCallback(StateChangedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_stateCallback = std::move(callback);
}

void TcpTransport::readWorker()
{
    std::array<std::uint8_t, 2048> buffer {};

    while (m_running.load()) {
#ifdef _WIN32
        WSAPOLLFD pfd {};
        pfd.fd = static_cast<SOCKET>(m_sockfd);
        pfd.events = POLLIN;
#else
        struct pollfd pfd { };
        pfd.fd = m_sockfd;
        pfd.events = POLLIN;
#endif

        const int ret = POLL_SOCKET(&pfd, 1, 50);
        if (ret > 0 && (pfd.revents & POLLIN)) {
            const auto bytesRead = ::recv(
#ifdef _WIN32
                static_cast<SOCKET>(m_sockfd), reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()),
                0
#else
                m_sockfd, buffer.data(), buffer.size(), 0
#endif
            );

            if (bytesRead > 0) {
                std::lock_guard<std::mutex> lock(m_callbackMutex);
                if (m_dataCallback) {
                    m_dataCallback(buffer.data(), static_cast<std::size_t>(bytesRead));
                }
            } else if (bytesRead == 0) {
                notifyState(TransportState::Disconnected, "Remote host closed connection");
                break;
            } else if (!IS_WOULDBLOCK()) {
                const std::string errStr = getSocketErrorString();
                notifyState(TransportState::Error, "Socket read error: " + errStr);
                break;
            }
        }
    }
}

void TcpTransport::notifyState(TransportState state, const std::string& errorMsg)
{
    StateChangedCallback cb;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        cb = m_stateCallback;
    }
    if (cb) {
        cb(state, errorMsg);
    }
}

} // namespace FujinonSX800

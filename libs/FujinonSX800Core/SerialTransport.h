#pragma once

/// @file SerialTransport.h
/// @brief Cross-platform serial port transport for Linux (termios) and Windows (Win32 API).

#include "BaseTransport.h"

#include <cstdint>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
using SerialHandle = HANDLE;
#define INVALID_SERIAL_HANDLE INVALID_HANDLE_VALUE
#else
using SerialHandle = int;
#define INVALID_SERIAL_HANDLE (-1)
#endif

namespace FujinonSX800 {

/// @class SerialTransport
/// @brief Cross-platform thread-safe serial transport (zero Qt dependency).
class SerialTransport : public BaseTransport {
public:
    explicit SerialTransport(std::string portName = "", std::uint32_t baudRate = 9600U);
    ~SerialTransport() override;

    // Non-copyable, non-movable
    SerialTransport(const SerialTransport&) = delete;
    SerialTransport& operator=(const SerialTransport&) = delete;
    SerialTransport(SerialTransport&&) = delete;
    SerialTransport& operator=(SerialTransport&&) = delete;

    void setPortName(const std::string& portName);
    [[nodiscard]] std::string getPortName() const;

    void setBaudRate(std::uint32_t baudRate);
    [[nodiscard]] std::uint32_t getBaudRate() const noexcept;

    // ITransport interface
    [[nodiscard]] bool open() override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] bool sendData(const std::vector<std::uint8_t>& data) override;

private:
    void readWorker();
    bool configurePort();

    std::string m_portName;
    std::uint32_t m_baudRate { 9600U };
    SerialHandle m_handle { INVALID_SERIAL_HANDLE };
};

} // namespace FujinonSX800

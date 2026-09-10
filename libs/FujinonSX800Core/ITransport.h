#pragma once

/// @file ITransport.h
/// @brief Abstract transport layer interface for Pelco-D communications (zero Qt dependency).

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace FujinonSX800 {

/// @enum TransportState
/// @brief Connection state of the physical or simulated transport channel.
enum class TransportState : std::uint8_t {
    Disconnected = 0x00U,
    Connecting = 0x01U,
    Connected = 0x02U,
    Error = 0x03U
};

/// @class ITransport
/// @brief Pure virtual interface defining raw byte streaming and lifecycle.
class ITransport {
public:
    using DataReceivedCallback = std::function<void(const std::vector<std::uint8_t>& data)>;
    using StateChangedCallback = std::function<void(TransportState state, const std::string& errorMsg)>;

    virtual ~ITransport() = default;

    /// @brief Opens the underlying physical or virtual channel.
    /// @return True on success, false on failure.
    [[nodiscard]] virtual bool open() = 0;

    /// @brief Closes the channel and cleans up handles.
    virtual void close() = 0;

    /// @brief Checks whether the channel is currently established and open.
    /// @return True if connected and ready for I/O.
    [[nodiscard]] virtual bool isOpen() const noexcept = 0;

    /// @brief Transmits raw bytes across the medium.
    /// @param[in] data Byte buffer to send.
    /// @return True if write succeeded.
    [[nodiscard]] virtual bool sendData(const std::vector<std::uint8_t>& data) = 0;

    /// @brief Registers callback for incoming bytes.
    /// @param[in] callback Function invoked when new data arrives.
    virtual void setDataCallback(DataReceivedCallback callback) = 0;

    /// @brief Registers callback for transport state changes.
    /// @param[in] callback Function invoked on connect/disconnect/error.
    virtual void setStateCallback(StateChangedCallback callback) = 0;
};

} // namespace FujinonSX800

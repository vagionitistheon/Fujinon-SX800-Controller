#pragma once

/// @file ProtocolParser.h
/// @brief Pelco-D response parser decoding incoming packets into CameraStatus telemetry.

#include "CameraStatus.h"
#include "PelcoDFrame.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace FujinonSX800 {

/// @class ProtocolParser
/// @brief Dispatches and decodes camera responses into the high-level CameraStatus model.
class ProtocolParser {
public:
    /// @brief Parses an incoming Pelco-D frame and updates the status structure.
    /// @param[in] packet Validated byte frame from camera.
    /// @param[in,out] status High-level status model updated with decoded fields.
    /// @param[in] lastQuery Optional hint about the last query dispatched (for 18-byte payloads).
    /// @return True if frame was recognized and decoded.
    [[nodiscard]] static bool parsePacket(
        const std::vector<std::uint8_t>& packet,
        CameraStatus& status,
        std::string_view lastQuery = {}) noexcept;

    /// @brief Generates human-readable description string for a Pelco-D frame.
    /// @param[in] frame Raw frame bytes.
    /// @param[in] isTx True if outbound command, false if inbound response.
    /// @return Descriptive string.
    [[nodiscard]] static std::string describeFrame(
        const std::vector<std::uint8_t>& frame, bool isTx = true);

private:
    static bool parseGeneralAck(
        const std::vector<std::uint8_t>& packet, CameraStatus& status) noexcept;

    static bool parseExtendedResponse(
        const std::vector<std::uint8_t>& packet, CameraStatus& status) noexcept;

    static bool parseQueryResponse(
        const std::vector<std::uint8_t>& packet,
        CameraStatus& status,
        std::string_view lastQuery) noexcept;
};

} // namespace FujinonSX800

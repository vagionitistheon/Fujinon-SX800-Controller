#pragma once

/// @file PelcoDFrame.h
/// @brief Pelco-D packet definition, serialization, checksum calculation, and stream splitting.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace FujinonSX800 {

/// @class PelcoDFrame
/// @brief Static utilities for Pelco-D frame creation, validation, and stream framing.
class PelcoDFrame {
public:
    static constexpr std::uint8_t SyncByte { 0xFFU };
    static constexpr std::size_t GeneralResponseSize { 4U };
    static constexpr std::size_t StandardFrameSize { 7U };
    static constexpr std::size_t QueryResponseSize { 18U };

    /// @brief Computes Pelco-D 8-bit checksum over payload bytes (sum modulo 256).
    /// @param[in] bytes Continuous buffer of payload bytes excluding sync byte.
    /// @return 8-bit checksum value.
    [[nodiscard]] static std::uint8_t calculateChecksum(const std::vector<std::uint8_t>& bytes) noexcept;

    /// @brief Overload computing checksum for a pointer range.
    /// @param[in] data Pointer to payload start.
    /// @param[in] length Number of bytes in payload.
    /// @return 8-bit checksum value.
    [[nodiscard]] static std::uint8_t calculateChecksum(const std::uint8_t* data, std::size_t length) noexcept;

    /// @brief Creates a 7-byte command frame.
    /// @param[in] address Camera RS-485 bus address (1 - 31).
    /// @param[in] cmd1 Command 1 byte.
    /// @param[in] cmd2 Command 2 byte.
    /// @param[in] data1 Data 1 byte.
    /// @param[in] data2 Data 2 byte.
    /// @return 7-byte Pelco-D formatted vector.
    [[nodiscard]] static std::vector<std::uint8_t> createFrame(
        std::uint8_t address, std::uint8_t cmd1, std::uint8_t cmd2, std::uint8_t data1, std::uint8_t data2);

    /// @brief Checks if a byte is a valid Pelco-D command or response opcode prefix.
    /// @param[in] cmd1 Command 1 or Response 1 byte (frame index 2).
    /// @return True if cmd1 is a recognized standard or extended opcode.
    [[nodiscard]] static bool isValidOpcode(std::uint8_t cmd1) noexcept;

    /// @brief Validates checksum for a complete frame (4-byte, 7-byte, or 18-byte).
    /// @param[in] frame Complete frame starting with 0xFF.
    /// @return True if length and checksum match protocol rules.
    [[nodiscard]] static bool isValidFrame(const std::vector<std::uint8_t>& frame) noexcept;

    /// @brief Splits contiguous byte stream into valid individual Pelco-D frames.
    /// @param[in] stream Raw input byte buffer.
    /// @return Collection of distinct verified packet frames.
    [[nodiscard]] static std::vector<std::vector<std::uint8_t>> splitStream(const std::vector<std::uint8_t>& stream);
};

} // namespace FujinonSX800

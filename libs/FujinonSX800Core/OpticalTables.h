#pragma once

/// @file OpticalTables.h
/// @brief Optical conversion tables for Fujinon SX800 zoom focal length, FOV, and focus distance.

#include <cstdint>
#include <vector>

namespace FujinonSX800 {

/// @struct OpticalEntry
/// @brief Entry in optical lookup tables.
struct OpticalEntry {
    std::uint16_t pulse { 0U };
    double value { 0.0 };
};

/// @class OpticalTables
/// @brief Converts between raw motor pulse steps and physical optical values.
class OpticalTables {
public:
    /// @brief Converts zoom pulse (0 - 65535) to focal length in millimeters (20.0 - 800.0).
    /// @param[in] pulse 16-bit zoom position.
    /// @return Focal length in mm.
    [[nodiscard]] static double pulseToFocalLength(std::uint16_t pulse) noexcept;

    /// @brief Converts zoom pulse (0 - 65535) to Horizontal Field of View in degrees.
    /// @param[in] pulse 16-bit zoom position.
    /// @return HFOV in degrees.
    [[nodiscard]] static double pulseToHorizontalFov(std::uint16_t pulse) noexcept;

    /// @brief Converts focal length in millimeters to closest 16-bit zoom pulse.
    /// @param[in] focalLengthMm Desired focal length in mm.
    /// @return 16-bit zoom pulse.
    [[nodiscard]] static std::uint16_t focalLengthToPulse(double focalLengthMm) noexcept;

    /// @brief Converts focus pulse (0 - 65535) to distance in meters (5.0m to 5000m/infinity).
    /// @param[in] pulse 16-bit focus position.
    /// @return Distance in meters.
    [[nodiscard]] static double pulseToDistance(std::uint16_t pulse) noexcept;

private:
    [[nodiscard]] static double interpolate(
        std::uint16_t pulse, const std::vector<OpticalEntry>& table) noexcept;

    [[nodiscard]] static std::uint16_t inverseInterpolate(
        double value, const std::vector<OpticalEntry>& table) noexcept;
};

} // namespace FujinonSX800

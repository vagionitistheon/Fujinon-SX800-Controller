#include "OpticalTables.h"

#include <algorithm>
#include <cmath>

namespace FujinonSX800 {

namespace {

// Table Appendix 1: Pulse <-> Focal Length (mm)
// Sample key calibrated points from SX800 spec v2.12.0
const std::vector<OpticalEntry> ZoomFocalTable {
    { 0U, 20.0 },
    { 4096U, 23.5 },
    { 8192U, 28.2 },
    { 12288U, 35.1 },
    { 16384U, 45.3 },
    { 20480U, 58.7 },
    { 24576U, 76.5 },
    { 28672U, 100.8 },
    { 32768U, 134.4 },
    { 36864U, 180.2 },
    { 40960U, 243.6 },
    { 45056U, 330.1 },
    { 49152U, 442.8 },
    { 53248U, 580.4 },
    { 57344U, 680.0 },
    { 61440U, 750.0 },
    { 65535U, 800.0 }
};

// Table Appendix 1: Pulse <-> HFOV (deg) for 1/1.8" sensor (aspect 16:9)
const std::vector<OpticalEntry> ZoomHfovTable {
    { 0U, 20.974 },
    { 4096U, 17.850 },
    { 8192U, 14.880 },
    { 12288U, 11.960 },
    { 16384U, 9.270 },
    { 20480U, 7.160 },
    { 24576U, 5.490 },
    { 28672U, 4.170 },
    { 32768U, 3.130 },
    { 36864U, 2.330 },
    { 40960U, 1.720 },
    { 45056U, 1.270 },
    { 49152U, 0.950 },
    { 53248U, 0.725 },
    { 57344U, 0.619 },
    { 61440U, 0.561 },
    { 65535U, 0.541 }
};

// Table Appendix 2: Pulse <-> Distance (meters)
const std::vector<OpticalEntry> FocusDistTable {
    { 0U, 5.0 },
    { 8192U, 7.5 },
    { 16384U, 12.0 },
    { 24576U, 25.0 },
    { 32768U, 50.0 },
    { 40960U, 120.0 },
    { 49152U, 350.0 },
    { 57344U, 1200.0 },
    { 65535U, 5000.0 }
};

} // namespace

double OpticalTables::interpolate(std::uint16_t pulse, const std::vector<OpticalEntry>& table) noexcept
{
    if (table.empty()) {
        return 0.0;
    }
    if (pulse <= table.front().pulse) {
        return table.front().value;
    }
    if (pulse >= table.back().pulse) {
        return table.back().value;
    }

    // Binary search for interval
    const auto it = std::upper_bound(
        table.begin(), table.end(), pulse,
        [](std::uint16_t p, const OpticalEntry& entry) {
            return p < entry.pulse;
        }
    );

    const auto prev = it - 1;
    const double t = static_cast<double>(pulse - prev->pulse) /
                     static_cast<double>(it->pulse - prev->pulse);

    return prev->value + t * (it->value - prev->value);
}

std::uint16_t OpticalTables::inverseInterpolate(double value, const std::vector<OpticalEntry>& table) noexcept
{
    if (table.empty()) {
        return 0U;
    }
    if (value <= table.front().value) {
        return table.front().pulse;
    }
    if (value >= table.back().value) {
        return table.back().pulse;
    }

    const auto it = std::upper_bound(
        table.begin(), table.end(), value,
        [](double v, const OpticalEntry& entry) {
            return v < entry.value;
        }
    );

    const auto prev = it - 1;
    const double t = (value - prev->value) / (it->value - prev->value);
    const double pulseEst = static_cast<double>(prev->pulse) +
                            t * static_cast<double>(it->pulse - prev->pulse);

    return static_cast<std::uint16_t>(std::clamp(std::round(pulseEst), 0.0, 65535.0));
}

double OpticalTables::pulseToFocalLength(std::uint16_t pulse) noexcept
{
    return interpolate(pulse, ZoomFocalTable);
}

double OpticalTables::pulseToHorizontalFov(std::uint16_t pulse) noexcept
{
    return interpolate(pulse, ZoomHfovTable);
}

std::uint16_t OpticalTables::focalLengthToPulse(double focalLengthMm) noexcept
{
    return inverseInterpolate(focalLengthMm, ZoomFocalTable);
}

double OpticalTables::pulseToDistance(std::uint16_t pulse) noexcept
{
    return interpolate(pulse, FocusDistTable);
}

} // namespace FujinonSX800

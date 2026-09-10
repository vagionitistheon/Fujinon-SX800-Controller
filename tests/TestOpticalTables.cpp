/// @file TestOpticalTables.cpp
/// @brief Unit tests for optical conversions and table interpolation.

#include "FujinonSX800Core/OpticalTables.h"

#include <cassert>
#include <cmath>
#include <iostream>

void testZoomConversions()
{
    // Pulse 0 -> 20.0 mm, ~20.974 deg
    const double flMin = FujinonSX800::OpticalTables::pulseToFocalLength(0U);
    assert(std::abs(flMin - 20.0) < 0.001);

    const double hfovMin = FujinonSX800::OpticalTables::pulseToHorizontalFov(0U);
    assert(std::abs(hfovMin - 20.974) < 0.001);

    // Pulse 65535 -> 800.0 mm, ~0.546 deg
    const double flMax = FujinonSX800::OpticalTables::pulseToFocalLength(65535U);
    assert(std::abs(flMax - 800.0) < 0.001);

    const double hfovMax = FujinonSX800::OpticalTables::pulseToHorizontalFov(65535U);
    assert(std::abs(hfovMax - 0.546) < 0.001);

    // VFOV & DFOV at Wide: 11.847 deg and 24.017 deg
    const double vfovMin = FujinonSX800::OpticalTables::pulseToVerticalFov(0U);
    assert(std::abs(vfovMin - 11.847) < 0.001);
    const double dfovMin = FujinonSX800::OpticalTables::pulseToDiagonalFov(0U);
    assert(std::abs(dfovMin - 24.017) < 0.001);

    // IFOV calculation at wide (20mm) and tele (800mm)
    const double ifovWide = FujinonSX800::OpticalTables::calculateIfovMrad(20.0);
    assert(ifovWide > 0.20 && ifovWide < 0.21);
    const double ifovTele = FujinonSX800::OpticalTables::calculateIfovMrad(800.0);
    assert(ifovTele > 0.004 && ifovTele < 0.006);

    // Monotonic increase of focal length
    double lastFl = 0.0;
    for (std::uint32_t p = 0; p <= 65535; p += 5000) {
        const double fl = FujinonSX800::OpticalTables::pulseToFocalLength(static_cast<std::uint16_t>(p));
        assert(fl >= lastFl);
        lastFl = fl;
    }

    // Inverse conversion: focal length to pulse
    const std::uint16_t pulse20 = FujinonSX800::OpticalTables::focalLengthToPulse(20.0);
    assert(pulse20 == 0U);

    const std::uint16_t pulse800 = FujinonSX800::OpticalTables::focalLengthToPulse(800.0);
    assert(pulse800 == 65535U);
}

void testFocusConversions()
{
    // Pulse 0 -> 5.0m
    const double distMin = FujinonSX800::OpticalTables::pulseToDistance(0U);
    assert(std::abs(distMin - 5.0) < 0.001);

    // Pulse 65535 -> 5000m (Infinity)
    const double distMax = FujinonSX800::OpticalTables::pulseToDistance(65535U);
    assert(std::abs(distMax - 5000.0) < 0.001);
}

int main()
{
    testZoomConversions();
    testFocusConversions();

    std::cout << "[PASS] TestOpticalTables completed successfully." << std::endl;
    return 0;
}

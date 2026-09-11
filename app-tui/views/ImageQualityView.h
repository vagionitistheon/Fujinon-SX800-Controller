#pragma once

/// @file ImageQualityView.h
/// @brief Image quality, fine calibration, and enhancement settings view for SX800 TUI.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "FujinonSX800Core/CameraStatus.h"

namespace FujinonSX800Tui {

/// @class ImageQualityView
/// @brief Renders adjustments for tone, color shifts, WDR, defog, and white balance.
class ImageQualityView {
public:
    ImageQualityView() = default;

    void render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom);
};

} // namespace FujinonSX800Tui

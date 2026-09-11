#pragma once

/// @file OpticsView.h
/// @brief Optics, zoom, focus, and iris telemetry and control view for SX800 TUI.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "FujinonSX800Core/CameraStatus.h"

namespace FujinonSX800Tui {

/// @class OpticsView
/// @brief Visualizes optical zoom gauges, field-of-view geometry, focus, and iris controls.
class OpticsView {
public:
    OpticsView() = default;

    void render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom);
};

} // namespace FujinonSX800Tui

#pragma once

/// @file DayNightView.h
/// @brief Day/Night switching and infrared optical filter controls for SX800 TUI.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "FujinonSX800Core/CameraStatus.h"

namespace FujinonSX800Tui {

/// @class DayNightView
/// @brief Visualizes optical IR cut filters, IR wavelength selection, and sensor sensitivity.
class DayNightView {
public:
    DayNightView() = default;

    void render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom);
};

} // namespace FujinonSX800Tui

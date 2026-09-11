#pragma once

/// @file SystemView.h
/// @brief Hardware identification, diagnostics, and photo preset management view for SX800 TUI.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "FujinonSX800Core/CameraStatus.h"

namespace FujinonSX800Tui {

/// @class SystemView
/// @brief Displays hardware serial number, firmware revision, presets, and maintenance triggers.
class SystemView {
public:
    SystemView() = default;

    void render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom);
};

} // namespace FujinonSX800Tui

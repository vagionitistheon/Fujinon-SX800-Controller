#pragma once

/// @file OsdVideoView.h
/// @brief OSD overlays, video standards, and RTC synchronization view for SX800 TUI.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "FujinonSX800Core/CameraStatus.h"

namespace FujinonSX800Tui {

/// @class OsdVideoView
/// @brief Renders OSD coordinate positions, video formats, and hardware RTC clocks.
class OsdVideoView {
public:
    OsdVideoView() = default;

    void render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom);
};

} // namespace FujinonSX800Tui

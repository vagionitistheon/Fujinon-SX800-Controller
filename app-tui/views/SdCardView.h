#pragma once

/// @file SdCardView.h
/// @brief SD card recording, media playback, and transport control view for SX800 TUI.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "FujinonSX800Core/CameraStatus.h"

namespace FujinonSX800Tui {

/// @class SdCardView
/// @brief Visualizes SD movie storage, record mode, capacity gauges, and playback transport.
class SdCardView {
public:
    SdCardView() = default;

    void render(Canvas& canvas, const FujinonSX800::CameraStatus& status, int top, int bottom);
};

} // namespace FujinonSX800Tui

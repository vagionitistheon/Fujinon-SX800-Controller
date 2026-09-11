#pragma once

/// @file HeaderView.h
/// @brief Top dashboard header displaying system status, thermal telemetry, and tab bar.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "FujinonSX800Core/CameraStatus.h"

#include <string>

namespace FujinonSX800Tui {

/// @class HeaderView
/// @brief Renders brand title, connection badges, temperature telemetry, and tab bar.
class HeaderView {
public:
    HeaderView() = default;

    void render(Canvas& canvas, const FujinonSX800::CameraStatus& status, bool isConnected, bool isMock,
        const std::string& connInfo, int activeTab);
};

} // namespace FujinonSX800Tui

#pragma once

/// @file FooterView.h
/// @brief Bottom navigation bar and status message ticker for the SX800 TUI.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"

#include <string>

namespace FujinonSX800Tui {

/// @class FooterView
/// @brief Displays keybinding guide and recent log message ticker.
class FooterView {
public:
    FooterView() = default;

    void render(Canvas& canvas, const std::string& statusMessage);
};

} // namespace FujinonSX800Tui

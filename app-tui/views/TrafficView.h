#pragma once

/// @file TrafficView.h
/// @brief Live Pelco-D serial/TCP frame inspector and protocol sniffer view.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace FujinonSX800Tui {

/// @struct LoggedFrame
/// @brief Captured raw Pelco-D packet metadata.
struct LoggedFrame {
    std::string timestamp {};
    bool isTx { false };
    std::vector<std::uint8_t> rawBytes {};
    std::string hexStr {};
    std::string description {};
};

/// @class TrafficView
/// @brief Real-time scrolling table displaying transmitted and received Pelco-D packets.
class TrafficView {
public:
    TrafficView() = default;

    void addFrame(const std::vector<std::uint8_t>& frame, bool isTx);
    void clear();

    void render(Canvas& canvas, int top, int bottom);

private:
    std::mutex m_mutex {};
    std::deque<LoggedFrame> m_frames {};
    static constexpr size_t MaxFrames { 100U };
};

} // namespace FujinonSX800Tui

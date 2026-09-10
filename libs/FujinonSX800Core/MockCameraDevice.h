#pragma once

/// @file MockCameraDevice.h
/// @brief In-memory SX800 camera emulator implementing ITransport for offline testing.

#include "ITransport.h"
#include "PelcoDTypes.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace FujinonSX800 {

/// @struct MockInternalState
/// @brief Simulated hardware state of the mock SX800 camera.
struct MockInternalState {
    std::uint16_t zoomPosition { 1000U };
    std::uint16_t focusPosition { 2000U };
    std::uint16_t irisPosition { 500U };
    DefogMode defogMode { DefogMode::Off };
    DeHeatHazeMode deHeatHazeMode { DeHeatHazeMode::Off };
    VlcFilterMode vlcFilterMode { VlcFilterMode::Off };
    OpticalStabilization oisMode { OpticalStabilization::Off };
    DayNightMode dayNightMode { DayNightMode::Day };
    std::uint8_t brightness { 128U };
    std::uint8_t contrast { 128U };
    std::uint8_t saturation { 128U };
    std::uint8_t sharpness { 128U };
    std::string serialNumber { "SX800999" };
    std::uint8_t fwMajor { 2U };
    std::uint8_t fwMinor { 12U };
};

/// @class MockCameraDevice
/// @brief In-memory mock transport executing the SX800 Pelco-D protocol state machine.
class MockCameraDevice : public ITransport {
public:
    explicit MockCameraDevice(std::uint8_t address = 1U) noexcept;
    ~MockCameraDevice() override = default;

    // Non-copyable, non-movable
    MockCameraDevice(const MockCameraDevice&) = delete;
    MockCameraDevice& operator=(const MockCameraDevice&) = delete;
    MockCameraDevice(MockCameraDevice&&) = delete;
    MockCameraDevice& operator=(MockCameraDevice&&) = delete;

    [[nodiscard]] MockInternalState getInternalState() const;
    void setInternalState(const MockInternalState& state);

    // ITransport interface
    [[nodiscard]] bool open() override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] bool sendData(const std::vector<std::uint8_t>& data) override;
    void setDataCallback(DataReceivedCallback callback) override;
    void setStateCallback(StateChangedCallback callback) override;

private:
    void processIncomingFrame(const std::vector<std::uint8_t>& frame);

    std::uint8_t m_address { 1U };
    std::atomic<bool> m_open { false };

    mutable std::mutex m_stateMutex;
    MockInternalState m_state;

    mutable std::mutex m_callbackMutex;
    DataReceivedCallback m_dataCallback;
    StateChangedCallback m_stateCallback;
};

} // namespace FujinonSX800

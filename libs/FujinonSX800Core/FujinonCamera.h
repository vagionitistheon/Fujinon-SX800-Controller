#pragma once

/// @file FujinonCamera.h
/// @brief Asynchronous thread-safe controller managing Pelco-D communication with SX800.

#include "CameraStatus.h"
#include "CircularByteRing.h"
#include "ITransport.h"
#include "PelcoDTypes.h"
#include "ProtocolBuilder.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace FujinonSX800 {

/// @class FujinonCamera
/// @brief High-level controller coordinating transports, telemetry polling, and command queuing.
class FujinonCamera {
public:
    using StatusCallback = std::function<void(const CameraStatus& status)>;
    using TrafficCallback = std::function<void(bool isTx, const std::vector<std::uint8_t>& frame)>;
    using TimeoutCallback = std::function<void(const std::string& queryTag)>;

    explicit FujinonCamera(std::shared_ptr<ITransport> transport, std::uint8_t address = 1U);
    ~FujinonCamera();

    // Non-copyable, non-movable
    FujinonCamera(const FujinonCamera&) = delete;
    FujinonCamera& operator=(const FujinonCamera&) = delete;
    FujinonCamera(FujinonCamera&&) = delete;
    FujinonCamera& operator=(FujinonCamera&&) = delete;

    [[nodiscard]] bool start();
    void stop();
    [[nodiscard]] bool isConnected() const noexcept;

    void setAddress(std::uint8_t address);
    [[nodiscard]] std::uint8_t getAddress() const noexcept;

    void addStatusCallback(StatusCallback cb);
    void addTrafficCallback(TrafficCallback cb);
    void addTimeoutCallback(TimeoutCallback cb);
    [[nodiscard]] CameraStatus getStatus() const;

    void setAutoQueryOnConnect(bool enable) noexcept;
    [[nodiscard]] bool getAutoQueryOnConnect() const noexcept;
    void setTelemetryPolling(bool enable, std::uint32_t intervalMs = 1000) noexcept;
    [[nodiscard]] bool getTelemetryPolling() const noexcept;
    void setQueryTimeoutMs(std::uint32_t timeoutMs) noexcept;
    [[nodiscard]] std::uint32_t getQueryTimeoutMs() const noexcept;

    // Direct Optics & PTZ controls
    void zoomTele();
    void zoomWide();
    void zoomStop();
    void focusNear();
    void focusFar();
    void focusStop();
    void irisOpen();
    void irisClose();
    void irisStop();
    void setZoomPosition(std::uint16_t pulse);
    void setFocusPosition(std::uint16_t pulse);
    void setIrisPosition(std::uint16_t pos);

    // Auto focus & Day/Night
    void setOnePushAf();
    void setAutoFocus(AutoFocusMode mode);
    void setAfArea(AfArea area);
    void setAfSensitivity(AfSensitivity sens);
    void setDayNightMode(DayNightMode mode);
    void setOpticalFilterDay(OpticalFilter filter);
    void setOpticalFilterNight(OpticalFilter filter);
    void setIrWavelength(IrWavelength wl);

    // Filters & Image quality
    void setVlcFilter(VlcFilterMode mode);
    void setDefog(DefogMode mode);
    void setDeHeatHaze(DeHeatHazeMode mode);
    void setOpticalStabilization(OpticalStabilization mode);
    void setBrightness(std::uint8_t val);
    void setContrast(std::uint8_t val);
    void setSaturation(std::uint8_t val);
    void setSharpness(std::uint8_t val);
    void setWhiteBalance(WhiteBalanceMode mode);
    void setColorTemperature(std::uint16_t kelvin);
    void setDigitalZoom(DigitalZoomMode mode, std::uint8_t mag);
    void setNoiseReduction(NoiseReductionLevel level);

    // Navigation, System & Maintenance
    void sendMenuKey(MenuKey key);
    void sendRawFrame(const std::vector<std::uint8_t>& frame);
    void sendQueryFrame(const std::vector<std::uint8_t>& frame, std::string queryTag = "CustomQuery");
    void queryAll();
    void queryTemperature();
    void setTermination(bool enable);
    void reboot();
    void factoryReset();

private:
    void enqueueCommand(const std::vector<std::uint8_t>& frame, std::string queryTag = "");
    void workerLoop();
    void pollingLoop();
    void rxLoop();
    void onDataReceived(const std::vector<std::uint8_t>& data);
    void dispatchFrame(const std::vector<std::uint8_t>& frame);
    void checkQueryTimeout();

    struct CommandItem {
        std::vector<std::uint8_t> frame;
        std::string queryTag;
    };

    std::shared_ptr<ITransport> m_transport;
    ProtocolBuilder m_builder;
    std::uint8_t m_address { 1U };

    std::atomic<bool> m_running { false };
    std::thread m_workerThread;
    std::thread m_pollThread;
    std::thread m_rxThread;

    std::mutex m_queueMutex;
    std::condition_variable m_queueCv;
    std::deque<CommandItem> m_commandQueue;

    CircularByteRing<65536> m_rxRing;
    std::mutex m_rxMutex;
    std::condition_variable m_rxCv;

    bool m_autoQueryOnConnect { false };
    bool m_telemetryPolling { false };
    std::uint32_t m_pollIntervalMs { 1000U };
    std::uint32_t m_queryTimeoutMs { 1000U };

    mutable std::mutex m_statusMutex;
    CameraStatus m_status;
    std::string m_lastQueryTag;

    std::atomic<bool> m_awaitingResponse { false };
    std::condition_variable m_responseCv;
    std::string m_pendingQueryTag;
    std::chrono::steady_clock::time_point m_querySentTime;

    mutable std::mutex m_callbackMutex;
    std::vector<StatusCallback> m_statusCallbacks;
    std::vector<TrafficCallback> m_trafficCallbacks;
    std::vector<TimeoutCallback> m_timeoutCallbacks;
};

} // namespace FujinonSX800

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
    using TransportStateCallback = std::function<void(TransportState state, const std::string& errorMsg)>;

    using CallbackId = std::uint64_t;
    static constexpr CallbackId InvalidCallbackId { 0U };

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

    CallbackId addStatusCallback(StatusCallback cb);
    CallbackId addTrafficCallback(TrafficCallback cb);
    CallbackId addTimeoutCallback(TimeoutCallback cb);
    CallbackId addTransportStateCallback(TransportStateCallback cb);

    bool removeCallback(CallbackId id);
    bool removeStatusCallback(CallbackId id);
    bool removeTrafficCallback(CallbackId id);
    bool removeTimeoutCallback(CallbackId id);
    bool removeTransportStateCallback(CallbackId id);
    void clearCallbacks();
    [[nodiscard]] CameraStatus getStatus() const;

    void setAutoQueryOnConnect(bool enable) noexcept;
    [[nodiscard]] bool getAutoQueryOnConnect() const noexcept;
    void setTelemetryPolling(bool enable, std::uint32_t intervalMs = 1000) noexcept;
    [[nodiscard]] bool getTelemetryPolling() const noexcept;
    void setQueryTimeoutMs(std::uint32_t timeoutMs) noexcept;
    [[nodiscard]] std::uint32_t getQueryTimeoutMs() const noexcept;
    void setAutoReconnect(bool enable) noexcept;
    [[nodiscard]] bool getAutoReconnect() const noexcept;

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
    void setManualIris(ManualIrisFNo fNo);
    void setShutterLimit(ShutterLimitMode limit);

    // Auto focus & Day/Night
    void setOnePushAf();
    void setAutoFocus(AutoFocusMode mode);
    void setAfArea(AfArea area);
    void setAfSensitivity(AfSensitivity sens);
    void setDayNightMode(DayNightMode mode);
    void setAutoDayNight(DayNightMode mode);
    void setDayNightTrigger(bool enable);
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

    // Video, OSD & Media
    void setVideoDisplayMode(VideoDisplayMode mode);
    void setVideoFormat(HdFormat format);
    void setOsdDatePosition(OsdPosition pos);
    void setOsdTitlePosition(OsdPosition pos);
    void setOsdIdPosition(OsdPosition pos);
    void setRtcTime(std::uint16_t year, std::uint8_t month, std::uint8_t day, std::uint8_t hour, std::uint8_t minute,
        std::uint8_t second);
    void setRtcTime(std::uint8_t year, std::uint8_t month, std::uint8_t day, std::uint8_t hour, std::uint8_t minute,
        std::uint8_t second);
    void setSdPlayback(SdPlaybackControl ctrl);
    void setSdPlaybackSpeed(SdPlaybackMode mode);
    void setSdMovieFile(std::uint16_t fileNo);

    // Navigation, System & Maintenance
    void sendMenuKey(MenuKey key);
    void sendMenuDirection(MenuDirection dir);
    void setPreset(PresetAction action, std::uint8_t presetId);
    void setBaudRate(BaudRate rate);
    void sendRawFrame(const std::vector<std::uint8_t>& frame);
    void sendQueryFrame(const std::vector<std::uint8_t>& frame, std::string queryTag = "CustomQuery");
    void queryAll();
    void queryTemperature();
    void queryManualIris();
    void setTermination(bool enable);
    void reboot();
    void factoryReset();
    [[nodiscard]] std::uint64_t rxOverflowDrops() const noexcept;

private:
    void enqueueCommand(const std::vector<std::uint8_t>& frame, std::string queryTag = "");
    void enqueueTelemetryQueries();
    void workerLoop();
    void reconnectLoop();
    void startReconnect();
    void rxLoop();
    void onDataReceived(const std::uint8_t* data, std::size_t size);
    void onTransportStateChanged(TransportState state, const std::string& errorMsg);
    void dispatchFrame(const std::vector<std::uint8_t>& frame);
    void checkQueryTimeout();
    [[nodiscard]] static bool is18ByteQuery(std::string_view tag) noexcept;

    struct CommandItem {
        std::vector<std::uint8_t> frame;
        std::string queryTag;
    };

    std::shared_ptr<ITransport> m_transport;
    ProtocolBuilder m_builder;
    std::atomic<std::uint8_t> m_address { 1U };

    std::atomic<bool> m_running { false };
    std::atomic<bool> m_connected { false };
    std::thread m_workerThread;
    std::thread m_rxThread;
    std::thread m_reconnectThread;

    std::mutex m_queueMutex;
    std::condition_variable m_queueCv;
    std::deque<CommandItem> m_commandQueue;

    CircularByteRing<65536> m_rxRing;
    std::mutex m_rxMutex;
    std::condition_variable m_rxCv;
    std::atomic<std::uint64_t> m_rxOverflowDrops { 0U };

    std::atomic<bool> m_autoQueryOnConnect { false };
    std::atomic<bool> m_telemetryPolling { false };
    std::atomic<std::uint32_t> m_pollIntervalMs { 1000U };
    std::atomic<std::uint32_t> m_queryTimeoutMs { 1000U };

    std::atomic<bool> m_autoReconnect { true };
    std::atomic<bool> m_reconnectActive { false };
    std::mutex m_reconnectMutex;
    std::condition_variable m_reconnectCv;

    mutable std::mutex m_statusMutex;
    CameraStatus m_status;
    std::string m_lastQueryTag;

    std::atomic<bool> m_awaitingResponse { false };
    std::condition_variable m_responseCv;
    std::string m_pendingQueryTag;
    std::chrono::steady_clock::time_point m_querySentTime;

    mutable std::mutex m_callbackMutex;
    std::atomic<CallbackId> m_nextCallbackId { 1U };
    std::vector<std::pair<CallbackId, StatusCallback>> m_statusCallbacks;
    std::vector<std::pair<CallbackId, TrafficCallback>> m_trafficCallbacks;
    std::vector<std::pair<CallbackId, TimeoutCallback>> m_timeoutCallbacks;
    std::vector<std::pair<CallbackId, TransportStateCallback>> m_transportStateCallbacks;
};

/// @class ScopedCallbackConnection
/// @brief RAII handle managing lifetime of a registered callback subscription.
class ScopedCallbackConnection {
public:
    ScopedCallbackConnection() = default;
    ScopedCallbackConnection(FujinonCamera* camera, FujinonCamera::CallbackId id) noexcept;
    ~ScopedCallbackConnection();

    ScopedCallbackConnection(ScopedCallbackConnection&& other) noexcept;
    ScopedCallbackConnection& operator=(ScopedCallbackConnection&& other) noexcept;

    ScopedCallbackConnection(const ScopedCallbackConnection&) = delete;
    ScopedCallbackConnection& operator=(const ScopedCallbackConnection&) = delete;

    void disconnect();
    [[nodiscard]] bool isConnected() const noexcept;
    [[nodiscard]] FujinonCamera::CallbackId id() const noexcept;

private:
    FujinonCamera* m_camera { nullptr };
    FujinonCamera::CallbackId m_id { FujinonCamera::InvalidCallbackId };
};

} // namespace FujinonSX800

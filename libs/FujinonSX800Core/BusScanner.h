#pragma once

/// @file BusScanner.h
/// @brief Asynchronous Pelco-D bus address discovery scanner.

#include "ITransport.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace FujinonSX800 {

/// @struct DiscoveredDevice
/// @brief Address and response details for a discovered bus device.
struct DiscoveredDevice {
    std::uint8_t address { 0U };
    std::uint32_t responseTimeMs { 0U };
    std::vector<std::uint8_t> rawResponse {};
};

/// @enum ScanState
/// @brief Operational state of the asynchronous scanner.
enum class ScanState : std::uint8_t { Idle, Scanning, Paused };

/// @struct ScanConfig
/// @brief Address range and timing configuration for a bus scan.
struct ScanConfig {
    std::uint8_t startAddress { 1U };
    std::uint8_t endAddress { 31U };
    std::uint32_t timeoutMs { 150U };
    std::uint32_t interCommandDelayMs { 20U };
};

/// @class BusScanner
/// @brief Probes a Pelco-D transport for responsive camera addresses.
class BusScanner {
public:
    using DeviceDiscoveredCallback = std::function<void(const DiscoveredDevice& device)>;
    using ScanProgressCallback = std::function<void(std::uint8_t address, std::size_t scanned, std::size_t total)>;
    using ScanStateChangedCallback = std::function<void(ScanState state)>;
    using ScanFinishedCallback = std::function<void(const std::vector<DiscoveredDevice>& devices)>;

    explicit BusScanner(std::shared_ptr<ITransport> transport = nullptr);
    ~BusScanner();

    BusScanner(const BusScanner&) = delete;
    BusScanner& operator=(const BusScanner&) = delete;
    BusScanner(BusScanner&&) = delete;
    BusScanner& operator=(BusScanner&&) = delete;

    void setTransport(std::shared_ptr<ITransport> transport);
    [[nodiscard]] std::shared_ptr<ITransport> getTransport() const;
    [[nodiscard]] bool startScan(const ScanConfig& config = {});
    void stopScan();
    void pauseScan();
    void resumeScan();
    [[nodiscard]] bool isScanning() const noexcept;
    [[nodiscard]] bool isPaused() const noexcept;
    [[nodiscard]] ScanState getState() const noexcept;
    [[nodiscard]] std::vector<DiscoveredDevice> getDiscoveredDevices() const;

    void setDeviceDiscoveredCallback(DeviceDiscoveredCallback callback);
    void setScanProgressCallback(ScanProgressCallback callback);
    void setScanStateChangedCallback(ScanStateChangedCallback callback);
    void setScanFinishedCallback(ScanFinishedCallback callback);

private:
    void scanWorker(ScanConfig config);
    void onDataReceived(const std::uint8_t* data, std::size_t size);
    void setState(ScanState state);

    mutable std::mutex m_mutex;
    std::shared_ptr<ITransport> m_transport;
    std::thread m_worker;
    std::atomic<bool> m_stopRequested { false };
    std::atomic<bool> m_pauseRequested { false };
    std::atomic<ScanState> m_state { ScanState::Idle };
    bool m_openedTransport { false };

    mutable std::mutex m_responseMutex;
    std::condition_variable m_responseCv;
    std::vector<std::uint8_t> m_responseBuffer;
    std::uint8_t m_probeAddress { 0U };
    bool m_responseReady { false };

    std::vector<DiscoveredDevice> m_discoveredDevices;
    DeviceDiscoveredCallback m_deviceCallback;
    ScanProgressCallback m_progressCallback;
    ScanStateChangedCallback m_stateCallback;
    ScanFinishedCallback m_finishedCallback;
};

} // namespace FujinonSX800

#pragma once

/// @file TuiApp.h
/// @brief Main application controller for the Fujinon SX800 TUI dashboard.
/// @author Theon Sensors / Maintainers

#include "Canvas.h"
#include "Terminal.h"
#include "views/ConnectionModal.h"
#include "views/DayNightView.h"
#include "views/FooterView.h"
#include "views/HeaderView.h"
#include "views/ImageQualityView.h"
#include "views/OpticsView.h"
#include "views/OsdVideoView.h"
#include "views/SdCardView.h"
#include "views/SystemView.h"
#include "views/TrafficView.h"

#include "FujinonSX800Core/CameraStatus.h"
#include "FujinonSX800Core/FujinonCamera.h"
#include "FujinonSX800Core/ITransport.h"
#include "FujinonSX800Core/MockCameraDevice.h"

#include <memory>
#include <mutex>
#include <string>

namespace FujinonSX800Tui {

/// @class TuiApp
/// @brief Application manager coordinating terminal I/O, views, and the camera controller.
class TuiApp {
public:
    TuiApp();
    ~TuiApp();

    TuiApp(const TuiApp&) = delete;
    TuiApp& operator=(const TuiApp&) = delete;
    TuiApp(TuiApp&&) = delete;
    TuiApp& operator=(TuiApp&&) = delete;

    [[nodiscard]] bool initMock(std::uint8_t address = 1U);
    [[nodiscard]] bool initSerial(const std::string& port, std::uint32_t baud, std::uint8_t address = 1U);
    [[nodiscard]] bool initTcp(const std::string& host, std::uint16_t port, std::uint8_t address = 1U);

    int run();
    void stop();

private:
    void handleInput(const KeyEvent& ev);
    void render();
    void setupCameraCallbacks();

    Terminal m_terminal {};
    Canvas m_canvas {};

    std::shared_ptr<FujinonSX800::ITransport> m_transport {};
    std::shared_ptr<FujinonSX800::MockCameraDevice> m_mockDevice {};
    std::unique_ptr<FujinonSX800::FujinonCamera> m_camera {};

    bool m_running { false };
    bool m_isMock { false };
    std::string m_connInfo {};
    std::string m_statusMsg { "TUI initialized" };

    int m_activeTab { 0 }; // 0: Optics, 1: ImgQ, 2: DayNight, 3: OsdVideo, 4: SdCard, 5: System, 6: Traffic

    std::mutex m_statusMutex {};
    FujinonSX800::CameraStatus m_status {};

    HeaderView m_headerView {};
    OpticsView m_opticsView {};
    ImageQualityView m_imageQualityView {};
    DayNightView m_dayNightView {};
    OsdVideoView m_osdVideoView {};
    SdCardView m_sdCardView {};
    SystemView m_systemView {};
    TrafficView m_trafficView {};
    FooterView m_footerView {};
    ConnectionModal m_connectionModal {};

    void switchConnection(
        ConnectionType type, const std::string& endpoint, std::uint32_t baudOrPort, std::uint8_t address);
};

} // namespace FujinonSX800Tui

#include "TuiApp.h"
#include "FujinonSX800Core/SerialTransport.h"
#include "FujinonSX800Core/TcpTransport.h"

#include <chrono>
#include <thread>

namespace FujinonSX800Tui {

TuiApp::TuiApp()
{
    m_connectionModal.setOnConnect(
        [this](ConnectionType type, const std::string& endpoint, std::uint32_t baudOrPort, std::uint8_t address) {
            switchConnection(type, endpoint, baudOrPort, address);
        });
}

TuiApp::~TuiApp()
{
    stop();
}

void TuiApp::setupCameraCallbacks()
{
    if (!m_camera) {
        return;
    }

    m_camera->addStatusCallback([this](const FujinonSX800::CameraStatus& status) {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_status = status;
    });

    m_camera->addTrafficCallback(
        [this](bool isTx, const std::vector<std::uint8_t>& frame) { m_trafficView.addFrame(frame, isTx); });

    m_camera->addTimeoutCallback([this](const std::string& queryTag) { m_statusMsg = "Query timeout: " + queryTag; });
}

bool TuiApp::initMock(std::uint8_t address)
{
    m_mockDevice = std::make_shared<FujinonSX800::MockCameraDevice>(address);
    m_transport = m_mockDevice;
    m_camera = std::make_unique<FujinonSX800::FujinonCamera>(m_transport, address);

    setupCameraCallbacks();
    m_camera->setAutoQueryOnConnect(true);
    m_camera->setTelemetryPolling(true, 500U);

    if (!m_camera->start()) {
        return false;
    }

    m_isMock = true;
    m_connInfo = "Virtual Simulator";
    m_statusMsg = "Connected to Mock SX800 Camera";
    return true;
}

bool TuiApp::initSerial(const std::string& port, std::uint32_t baud, std::uint8_t address)
{
    m_transport = std::make_shared<FujinonSX800::SerialTransport>(port, baud);
    m_camera = std::make_unique<FujinonSX800::FujinonCamera>(m_transport, address);

    setupCameraCallbacks();
    m_camera->setAutoQueryOnConnect(true);
    m_camera->setTelemetryPolling(true, 500U);

    if (!m_camera->start()) {
        return false;
    }

    m_isMock = false;
    m_connInfo = port + " @" + std::to_string(baud);
    m_statusMsg = "Connected via Serial (" + port + ")";
    return true;
}

bool TuiApp::initTcp(const std::string& host, std::uint16_t port, std::uint8_t address)
{
    m_transport = std::make_shared<FujinonSX800::TcpTransport>(host, port);
    m_camera = std::make_unique<FujinonSX800::FujinonCamera>(m_transport, address);

    setupCameraCallbacks();
    m_camera->setAutoQueryOnConnect(true);
    m_camera->setTelemetryPolling(true, 500U);

    if (!m_camera->start()) {
        return false;
    }

    m_isMock = false;
    m_connInfo = host + ":" + std::to_string(port);
    m_statusMsg = "Connected via TCP (" + m_connInfo + ")";
    return true;
}

void TuiApp::stop()
{
    m_running = false;
    if (m_camera) {
        m_camera->stop();
    }
    m_terminal.shutdown();
}

void TuiApp::switchConnection(
    ConnectionType type, const std::string& endpoint, std::uint32_t baudOrPort, std::uint8_t address)
{
    if (m_camera) {
        m_camera->stop();
        m_camera.reset();
    }
    m_transport.reset();
    m_mockDevice.reset();

    bool ok = false;
    switch (type) {
    case ConnectionType::Serial:
        ok = initSerial(endpoint, baudOrPort, address);
        break;
    case ConnectionType::Tcp:
        ok = initTcp(endpoint, static_cast<std::uint16_t>(baudOrPort), address);
        break;
    case ConnectionType::Mock:
        ok = initMock(address);
        break;
    }

    if (!ok) {
        m_statusMsg = "Connection failed! Reverting to virtual mock device";
        static_cast<void>(initMock(address));
    }
}

void TuiApp::handleInput(const KeyEvent& ev)
{
    // If modal is active, delegate all input to modal
    if (m_connectionModal.isOpen()) {
        m_connectionModal.handleInput(ev);
        return;
    }

    // Quit
    if (ev.isChar('q') || ev.isChar('Q') || (ev.key == Key::Char && ev.ctrl && ev.ch == 'c')) {
        m_running = false;
        return;
    }

    // Direct tab selection 1 .. 7
    if (ev.key == Key::Char && ev.ch >= '1' && ev.ch <= '7') {
        m_activeTab = ev.ch - '1';
        return;
    }

    // Tab cycling
    if (ev.key == Key::Tab) {
        m_activeTab = (m_activeTab + 1) % 7;
        return;
    }
    if (ev.key == Key::BackTab) {
        m_activeTab = (m_activeTab + 6) % 7;
        return;
    }

    // Connection Modal trigger [C]
    if (ev.isChar('c') || ev.isChar('C')) {
        const auto connType = m_isMock                    ? ConnectionType::Mock
            : (m_connInfo.find(':') != std::string::npos) ? ConnectionType::Tcp
                                                          : ConnectionType::Serial;
        m_connectionModal.setDefaults(connType, m_connInfo, m_status.rs485Address ? m_status.rs485Address : 7U);
        m_connectionModal.open();
        return;
    }

    if (!m_camera) {
        return;
    }

    // Motion & Camera Control Shortcuts
    if (ev.isChar('z') || ev.isChar('Z')) {
        m_camera->zoomTele();
        m_statusMsg = "Command: Zoom Tele (In)";
    } else if (ev.isChar('x') || ev.isChar('X')) {
        m_camera->zoomWide();
        m_statusMsg = "Command: Zoom Wide (Out)";
    } else if (ev.isChar('s') || ev.isChar('S') || ev.key == Key::Space) {
        m_camera->zoomStop();
        m_camera->focusStop();
        m_statusMsg = "Command: Optical Motion Stopped";
    } else if (ev.isChar('f') || ev.isChar('F')) {
        m_camera->focusNear();
        m_statusMsg = "Command: Focus Near";
    } else if (ev.isChar('g') || ev.isChar('G')) {
        m_camera->focusFar();
        m_statusMsg = "Command: Focus Far";
    } else if (ev.isChar('a') || ev.isChar('A')) {
        m_camera->setOnePushAf();
        m_statusMsg = "Command: One-Push AF Triggered";
    } else if (ev.isChar('o') || ev.isChar('O')) {
        const auto cur = m_status.opticalStabilization;
        const auto next = (cur == FujinonSX800::OpticalStabilization::Off) ? FujinonSX800::OpticalStabilization::OIS
                                                                           : FujinonSX800::OpticalStabilization::Off;
        m_camera->setOpticalStabilization(next);
        m_statusMsg = "Command: Toggled OIS Stabilizer";
    } else if (ev.isChar('i') || ev.isChar('I')) {
        m_camera->setIrisPosition(0x2000U);
        m_statusMsg = "Command: Set Nominal Iris Position";
    } else if (ev.isChar('r') || ev.isChar('R')) {
        m_camera->queryAll();
        m_statusMsg = "Command: Queried Complete Camera Telemetry";
    } else if (ev.isChar('k') || ev.isChar('K')) {
        m_trafficView.clear();
        m_statusMsg = "Inspector traffic buffer cleared";
    } else if (ev.isChar('m') || ev.isChar('M')) {
        if (!m_isMock) {
            if (m_camera) {
                m_camera->stop();
            }
            static_cast<void>(initMock(m_status.rs485Address ? m_status.rs485Address : 7U));
        }
    }
}

void TuiApp::render()
{
    int termW = 80;
    int termH = 24;
    static_cast<void>(m_terminal.getSize(termW, termH));

    if (m_terminal.checkResized() || m_canvas.width() != termW || m_canvas.height() != termH) {
        m_canvas.resize(termW, termH);
    }

    m_canvas.clear(Palette::DarkBg);

    FujinonSX800::CameraStatus currentStatus {};
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        currentStatus = m_status;
    }

    const bool connected = m_camera && m_camera->isConnected();

    // Render persistent Header (rows 0 .. 2)
    m_headerView.render(m_canvas, currentStatus, connected, m_isMock, m_connInfo, m_activeTab);

    // Render active Tab view (rows 3 .. termH - 3)
    const int tabTop = 3;
    const int tabBottom = termH - 2;

    switch (m_activeTab) {
    case 0:
        m_opticsView.render(m_canvas, currentStatus, tabTop, tabBottom);
        break;
    case 1:
        m_imageQualityView.render(m_canvas, currentStatus, tabTop, tabBottom);
        break;
    case 2:
        m_dayNightView.render(m_canvas, currentStatus, tabTop, tabBottom);
        break;
    case 3:
        m_osdVideoView.render(m_canvas, currentStatus, tabTop, tabBottom);
        break;
    case 4:
        m_sdCardView.render(m_canvas, currentStatus, tabTop, tabBottom);
        break;
    case 5:
        m_systemView.render(m_canvas, currentStatus, tabTop, tabBottom);
        break;
    case 6:
        m_trafficView.render(m_canvas, tabTop, tabBottom);
        break;
    default:
        break;
    }

    // Render persistent Footer (rows termH - 2 .. termH - 1)
    m_footerView.render(m_canvas, m_statusMsg);

    // Render Connection modal overlay if open
    if (m_connectionModal.isOpen()) {
        m_connectionModal.render(m_canvas);
    }

    // Delta-render to terminal
    m_canvas.render(m_terminal);
}

int TuiApp::run()
{
    if (!m_terminal.initialize()) {
        return 1;
    }

    m_running = true;

    while (m_running) {
        while (auto keyEvent = m_terminal.pollKey()) {
            handleInput(*keyEvent);
            if (!m_running) {
                break;
            }
        }

        render();

        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS loop
    }

    m_terminal.shutdown();
    return 0;
}

} // namespace FujinonSX800Tui

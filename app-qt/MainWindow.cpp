/// @file MainWindow.cpp
/// @brief Implementation of top-level application window.

#include "app-qt/MainWindow.h"

#include <QStatusBar>
#include <QToolBar>

namespace FujinonSX800App {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , camera { new FujinonSX800Qt::QFujinonCamera(nullptr, 7U, this) }
{
    setupUi();
    setupConnections();

    setWindowTitle(tr("Fujinon SX800 Surveillance Controller - Pelco-D v2.12.0"));
    resize(1200, 780);
    statusBar()->showMessage(tr("Ready. Connect to physical camera or launch Mock Simulator."));
}

void MainWindow::setupUi()
{
    // Toolbar with Connection Widget
    auto* toolBar = addToolBar(tr("Connection Toolbar"));
    toolBar->setMovable(false);
    connectionWidget = new ConnectionWidget(this);
    toolBar->addWidget(connectionWidget);

    // Central Tab Widget
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("centralWidget");

    opticsTab = new OpticsTab(camera, this);
    imageQualityTab = new ImageQualityTab(camera, this);
    dayNightTab = new DayNightTab(camera, this);
    osdVideoTab = new OsdVideoTab(camera, this);
    sdCardMenuTab = new SdCardMenuTab(camera, this);
    systemTab = new SystemTab(camera, this);

    tabWidget->addTab(opticsTab, tr("Optics & Motion"));
    tabWidget->addTab(imageQualityTab, tr("Image Quality & Filters"));
    tabWidget->addTab(dayNightTab, tr("Day / Night & Scheduling"));
    tabWidget->addTab(osdVideoTab, tr("OSD & Video"));
    tabWidget->addTab(sdCardMenuTab, tr("SD Card & Menu Remote"));
    tabWidget->addTab(systemTab, tr("System Diagnostics"));

    setCentralWidget(tabWidget);

    // Bottom Dock Widget for Protocol Inspector
    dockInspector = new QDockWidget(tr("Protocol Traffic Inspector & Injector"), this);
    dockInspector->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    dockInspector->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    inspectorWidget = new TrafficInspectorWidget(dockInspector);
    dockInspector->setWidget(inspectorWidget);
    addDockWidget(Qt::BottomDockWidgetArea, dockInspector);
}

void MainWindow::setupConnections()
{
    connect(connectionWidget, &ConnectionWidget::connectRequested, this, &MainWindow::handleConnect);
    connect(connectionWidget, &ConnectionWidget::disconnectRequested, this, &MainWindow::handleDisconnect);

    connect(camera, &FujinonSX800Qt::QFujinonCamera::statusUpdated, this, &MainWindow::handleStatusUpdated);
    connect(camera, &FujinonSX800Qt::QFujinonCamera::statusUpdated, opticsTab, &OpticsTab::updateTelemetry);
    connect(camera, &FujinonSX800Qt::QFujinonCamera::statusUpdated, imageQualityTab, &ImageQualityTab::updateTelemetry);
    connect(camera, &FujinonSX800Qt::QFujinonCamera::statusUpdated, dayNightTab, &DayNightTab::updateTelemetry);
    connect(camera, &FujinonSX800Qt::QFujinonCamera::statusUpdated, osdVideoTab, &OsdVideoTab::updateTelemetry);
    connect(camera, &FujinonSX800Qt::QFujinonCamera::statusUpdated, sdCardMenuTab, &SdCardMenuTab::updateTelemetry);
    connect(camera, &FujinonSX800Qt::QFujinonCamera::statusUpdated, systemTab, &SystemTab::updateTelemetry);

    connect(camera, &FujinonSX800Qt::QFujinonCamera::frameLogged, inspectorWidget, &TrafficInspectorWidget::logFrame);
    connect(inspectorWidget, &TrafficInspectorWidget::sendRawHexRequested, camera,
        &FujinonSX800Qt::QFujinonCamera::sendRawHex);

    connect(camera, &FujinonSX800Qt::QFujinonCamera::queryTimeoutOccurred, this, [this](const QString& tag) {
        const QString warnMsg = tr("Warning: Timeout waiting for response to '%1'").arg(tag);
        statusBar()->showMessage(warnMsg, 5000);
    });
}

void MainWindow::handleConnect(std::shared_ptr<FujinonSX800::ITransport> transport, std::uint8_t address)
{
    camera->setTransport(transport, address);
    const bool ok = camera->start();
    connectionWidget->setConnectionState(ok);

    if (ok) {
        statusBar()->showMessage(tr("Connected to camera (ID: %1).").arg(address));
    } else {
        statusBar()->showMessage(tr("Failed to connect to transport device."));
    }
}

void MainWindow::handleDisconnect()
{
    camera->stop();
    connectionWidget->setConnectionState(false);
    statusBar()->showMessage(tr("Disconnected."));
}

void MainWindow::handleStatusUpdated(const FujinonSX800::CameraStatus& status)
{
    const QString msg = tr("Online | Zoom: %1 mm (FOV %2°) | Focus: %3 m | Color Temp: %4 K")
                            .arg(status.focalLengthMm, 0, 'f', 1)
                            .arg(status.horizontalFovDeg, 0, 'f', 1)
                            .arg(status.focusDistanceM, 0, 'f', 1)
                            .arg(status.colorTemperatureKelvin);
    statusBar()->showMessage(msg);
}

} // namespace FujinonSX800App

#pragma once

/// @file MainWindow.h
/// @brief Main application window hosting dashboard tabs and traffic inspector.

#include "QFujinonCamera.h"
#include "app-qt/tabs/DayNightTab.h"
#include "app-qt/tabs/ImageQualityTab.h"
#include "app-qt/tabs/OpticsTab.h"
#include "app-qt/tabs/OsdVideoTab.h"
#include "app-qt/tabs/SdCardMenuTab.h"
#include "app-qt/tabs/SystemTab.h"
#include "app-qt/widgets/ConnectionWidget.h"
#include "app-qt/widgets/TrafficInspectorWidget.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QTabWidget>

namespace FujinonSX800App {

/// @class MainWindow
/// @brief Top-level application window for Fujinon SX800 Camera Control.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void handleConnect(std::shared_ptr<FujinonSX800::ITransport> transport, std::uint8_t address);
    void handleDisconnect();
    void handleStatusUpdated(const FujinonSX800::CameraStatus& status);

private:
    void setupUi();
    void setupConnections();

    FujinonSX800Qt::QFujinonCamera* camera { nullptr };

    ConnectionWidget* connectionWidget { nullptr };
    QTabWidget* tabWidget { nullptr };

    OpticsTab* opticsTab { nullptr };
    ImageQualityTab* imageQualityTab { nullptr };
    DayNightTab* dayNightTab { nullptr };
    OsdVideoTab* osdVideoTab { nullptr };
    SdCardMenuTab* sdCardMenuTab { nullptr };
    SystemTab* systemTab { nullptr };

    QDockWidget* dockInspector { nullptr };
    TrafficInspectorWidget* inspectorWidget { nullptr };
};

} // namespace FujinonSX800App

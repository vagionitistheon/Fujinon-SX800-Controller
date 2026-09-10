#pragma once

/// @file SystemTab.h
/// @brief UI Tab for camera hardware diagnostics, system configuration, and maintenance.

#include "CameraStatus.h"
#include "QFujinonCamera.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

namespace FujinonSX800App {

/// @class SystemTab
/// @brief Telemetry dashboard, hardware limits, and system maintenance utilities.
class SystemTab : public QWidget {
    Q_OBJECT

public:
    explicit SystemTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent = nullptr);
    ~SystemTab() override = default;

public slots:
    void updateTelemetry(const FujinonSX800::CameraStatus& status);

private:
    void setupUi();
    void setupConnections();

    FujinonSX800Qt::QFujinonCamera* cam { nullptr };

    // Telemetry Labels
    QLabel* lblSerialVal { nullptr };
    QLabel* lblFwVal { nullptr };
    QLabel* lblAddressVal { nullptr };
    QLabel* lblTempVal { nullptr };

    // Lens Status Indicators
    QLabel* lblIrisMoving { nullptr };
    QLabel* lblFocusMoving { nullptr };
    QLabel* lblZoomMoving { nullptr };
    QLabel* lblIrisLimits { nullptr };
    QLabel* lblFocusLimits { nullptr };
    QLabel* lblZoomLimits { nullptr };

    // System Settings
    QCheckBox* chkTermination { nullptr };
    QComboBox* cmbLanguage { nullptr };

    // Actions
    QPushButton* btnRefreshAll { nullptr };
    QCheckBox* chkContinuousPolling { nullptr };
    QSpinBox* spinPollInterval { nullptr };
    QPushButton* btnResetDefaults { nullptr };
    QPushButton* btnReboot { nullptr };
};

} // namespace FujinonSX800App

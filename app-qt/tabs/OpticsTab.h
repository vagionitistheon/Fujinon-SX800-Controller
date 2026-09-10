#pragma once

/// @file OpticsTab.h
/// @brief UI Tab for primary optics: Zoom, Focus, Iris, and Exposure controls.

#include "CameraStatus.h"
#include "QFujinonCamera.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

namespace FujinonSX800App {

/// @class OpticsTab
/// @brief Control interface for lens mechanisms and exposure parameters.
class OpticsTab : public QWidget {
    Q_OBJECT

public:
    explicit OpticsTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent = nullptr);
    ~OpticsTab() override = default;

public slots:
    void updateTelemetry(const FujinonSX800::CameraStatus& status);

private:
    void setupUi();
    void setupConnections();

    FujinonSX800Qt::QFujinonCamera* cam { nullptr };

    // Zoom Controls
    QPushButton* btnZoomTele { nullptr };
    QPushButton* btnZoomWide { nullptr };
    QSlider* sliderZoomPos { nullptr };
    QSpinBox* spinZoomPos { nullptr };
    QLabel* lblFocalLength { nullptr };
    QLabel* lblHfov { nullptr };
    QSlider* sliderZoomSpeed { nullptr };
    QLabel* lblZoomSpeedVal { nullptr };
    QComboBox* cmbDigitalZoom { nullptr };
    QComboBox* cmbDigitalZoomStep { nullptr };

    // Focus Controls
    QPushButton* btnFocusNear { nullptr };
    QPushButton* btnFocusFar { nullptr };
    QPushButton* btnPushToFocus { nullptr };
    QCheckBox* chkAutoFocus { nullptr };
    QSlider* sliderFocusPos { nullptr };
    QSpinBox* spinFocusPos { nullptr };
    QLabel* lblDistance { nullptr };
    QSlider* sliderFocusSpeed { nullptr };
    QLabel* lblFocusSpeedVal { nullptr };
    QComboBox* cmbAfArea { nullptr };
    QComboBox* cmbAfSensitivity { nullptr };

    // Iris & Exposure Controls
    QCheckBox* chkAutoIris { nullptr };
    QSlider* sliderIris { nullptr };
    QCheckBox* chkAgc { nullptr };
    QCheckBox* chkBlc { nullptr };
    QSpinBox* spinShutter { nullptr };
    QSpinBox* spinIso { nullptr };

    bool isUpdatingFromTelemetry { false };
};

} // namespace FujinonSX800App

#pragma once

/// @file ImageQualityTab.h
/// @brief UI Tab for image quality adjustments, filters, defog, haze, and white balance.

#include "CameraStatus.h"
#include "QFujinonCamera.h"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>

namespace FujinonSX800App {

/// @class ImageQualityTab
/// @brief Controls sensor processing, digital filters, and color calibration.
class ImageQualityTab : public QWidget {
    Q_OBJECT

public:
    explicit ImageQualityTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent = nullptr);
    ~ImageQualityTab() override = default;

public slots:
    void updateTelemetry(const FujinonSX800::CameraStatus& status);

private:
    void setupUi();
    void setupConnections();

    FujinonSX800Qt::QFujinonCamera* cam { nullptr };

    // Standard & Fine Sliders
    QSlider* sliderBrightness { nullptr };
    QSlider* sliderBrightnessFine { nullptr };
    QLabel* lblBrightnessVal { nullptr };

    QSlider* sliderContrast { nullptr };
    QSlider* sliderContrastFine { nullptr };
    QLabel* lblContrastVal { nullptr };

    QSlider* sliderSaturation { nullptr };
    QSlider* sliderSaturationFine { nullptr };
    QLabel* lblSaturationVal { nullptr };

    QSlider* sliderSharpness { nullptr };
    QSlider* sliderSharpnessFine { nullptr };
    QLabel* lblSharpnessVal { nullptr };

    // Optical & Sensor Filters
    QComboBox* cmbVlcFilter { nullptr };
    QComboBox* cmbDefog { nullptr };
    QComboBox* cmbDeHeatHaze { nullptr };
    QComboBox* cmbStabilization { nullptr };
    QComboBox* cmbWdr { nullptr };
    QComboBox* cmbNoiseReduction { nullptr };

    // White Balance
    QComboBox* cmbWbMode { nullptr };
    QSlider* sliderColorTemp { nullptr };
    QLabel* lblColorTempVal { nullptr };
    QSlider* sliderWbShiftRed { nullptr };
    QSlider* sliderWbShiftBlue { nullptr };
    QLabel* lblWbShiftRedVal { nullptr };
    QLabel* lblWbShiftBlueVal { nullptr };

    bool isUpdatingFromTelemetry { false };
};

} // namespace FujinonSX800App

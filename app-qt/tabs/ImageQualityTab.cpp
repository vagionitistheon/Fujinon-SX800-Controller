/// @file ImageQualityTab.cpp
/// @brief Implementation of image enhancement and color adjustments tab.

#include "ImageQualityTab.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace FujinonSX800App {

ImageQualityTab::ImageQualityTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent)
    : QWidget(parent)
    , cam { camera }
{
    setupUi();
    setupConnections();
}

void ImageQualityTab::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ==================== 1. IMAGE TUNING ====================
    auto* grpLevels = new QGroupBox(tr("Sensor Image Tuning (Standard & Fine)"), this);
    auto* levelsLayout = new QGridLayout(grpLevels);
    levelsLayout->setSpacing(10);

    // Brightness
    levelsLayout->addWidget(new QLabel(tr("Brightness (1..21):"), grpLevels), 0, 0);
    sliderBrightness = new QSlider(Qt::Horizontal, grpLevels);
    sliderBrightness->setRange(1, 21);
    sliderBrightness->setValue(10);
    levelsLayout->addWidget(sliderBrightness, 0, 1);

    sliderBrightnessFine = new QSlider(Qt::Horizontal, grpLevels);
    sliderBrightnessFine->setRange(-128, 127);
    sliderBrightnessFine->setValue(0);
    sliderBrightnessFine->setToolTip(tr("Fine Adjustment (-128..+127)"));
    levelsLayout->addWidget(sliderBrightnessFine, 0, 2);

    lblBrightnessVal = new QLabel("10 (0)", grpLevels);
    lblBrightnessVal->setObjectName("lblTelemetry");
    levelsLayout->addWidget(lblBrightnessVal, 0, 3);

    // Contrast
    levelsLayout->addWidget(new QLabel(tr("Contrast (1..5):"), grpLevels), 1, 0);
    sliderContrast = new QSlider(Qt::Horizontal, grpLevels);
    sliderContrast->setRange(1, 5);
    sliderContrast->setValue(3);
    levelsLayout->addWidget(sliderContrast, 1, 1);

    sliderContrastFine = new QSlider(Qt::Horizontal, grpLevels);
    sliderContrastFine->setRange(-128, 127);
    sliderContrastFine->setValue(0);
    levelsLayout->addWidget(sliderContrastFine, 1, 2);

    lblContrastVal = new QLabel("3 (0)", grpLevels);
    lblContrastVal->setObjectName("lblTelemetry");
    levelsLayout->addWidget(lblContrastVal, 1, 3);

    // Saturation
    levelsLayout->addWidget(new QLabel(tr("Saturation (1..5):"), grpLevels), 2, 0);
    sliderSaturation = new QSlider(Qt::Horizontal, grpLevels);
    sliderSaturation->setRange(1, 5);
    sliderSaturation->setValue(3);
    levelsLayout->addWidget(sliderSaturation, 2, 1);

    sliderSaturationFine = new QSlider(Qt::Horizontal, grpLevels);
    sliderSaturationFine->setRange(-128, 127);
    sliderSaturationFine->setValue(0);
    levelsLayout->addWidget(sliderSaturationFine, 2, 2);

    lblSaturationVal = new QLabel("3 (0)", grpLevels);
    lblSaturationVal->setObjectName("lblTelemetry");
    levelsLayout->addWidget(lblSaturationVal, 2, 3);

    // Sharpness
    levelsLayout->addWidget(new QLabel(tr("Sharpness (1..5):"), grpLevels), 3, 0);
    sliderSharpness = new QSlider(Qt::Horizontal, grpLevels);
    sliderSharpness->setRange(1, 5);
    sliderSharpness->setValue(3);
    levelsLayout->addWidget(sliderSharpness, 3, 1);

    sliderSharpnessFine = new QSlider(Qt::Horizontal, grpLevels);
    sliderSharpnessFine->setRange(-128, 127);
    sliderSharpnessFine->setValue(0);
    levelsLayout->addWidget(sliderSharpnessFine, 3, 2);

    lblSharpnessVal = new QLabel("3 (0)", grpLevels);
    lblSharpnessVal->setObjectName("lblTelemetry");
    levelsLayout->addWidget(lblSharpnessVal, 3, 3);

    mainLayout->addWidget(grpLevels);

    // ==================== 2. FILTERS & ENHANCEMENT ====================
    auto* grpFilters = new QGroupBox(tr("Optical Filters & Enhancement"), this);
    auto* filtersLayout = new QGridLayout(grpFilters);
    filtersLayout->setSpacing(10);

    // VLC Filter
    filtersLayout->addWidget(new QLabel(tr("VLC Filter:"), grpFilters), 0, 0);
    cmbVlcFilter = new QComboBox(grpFilters);
    cmbVlcFilter->addItem(tr("Off (Visible)"), static_cast<int>(FujinonSX800::VlcFilterMode::Off));
    cmbVlcFilter->addItem(tr("On (Cut Visible)"), static_cast<int>(FujinonSX800::VlcFilterMode::On));
    cmbVlcFilter->addItem(tr("Filter 4 (Custom)"), static_cast<int>(FujinonSX800::VlcFilterMode::Filter4));
    filtersLayout->addWidget(cmbVlcFilter, 0, 1);

    // Defog
    filtersLayout->addWidget(new QLabel(tr("Optical Defog:"), grpFilters), 1, 0);
    cmbDefog = new QComboBox(grpFilters);
    cmbDefog->addItem(tr("Off"), static_cast<int>(FujinonSX800::DefogMode::Off));
    cmbDefog->addItem(tr("Low"), static_cast<int>(FujinonSX800::DefogMode::Low));
    cmbDefog->addItem(tr("Mid"), static_cast<int>(FujinonSX800::DefogMode::Mid));
    cmbDefog->addItem(tr("High"), static_cast<int>(FujinonSX800::DefogMode::High));
    filtersLayout->addWidget(cmbDefog, 1, 1);

    // De-Heat Haze
    filtersLayout->addWidget(new QLabel(tr("De-Heat Haze:"), grpFilters), 2, 0);
    cmbDeHeatHaze = new QComboBox(grpFilters);
    cmbDeHeatHaze->addItem(tr("Off"), static_cast<int>(FujinonSX800::DeHeatHazeMode::Off));
    cmbDeHeatHaze->addItem(tr("Low"), static_cast<int>(FujinonSX800::DeHeatHazeMode::Low));
    cmbDeHeatHaze->addItem(tr("Mid"), static_cast<int>(FujinonSX800::DeHeatHazeMode::Mid));
    cmbDeHeatHaze->addItem(tr("High"), static_cast<int>(FujinonSX800::DeHeatHazeMode::High));
    filtersLayout->addWidget(cmbDeHeatHaze, 2, 1);

    // Stabilization
    filtersLayout->addWidget(new QLabel(tr("Stabilization:"), grpFilters), 3, 0);
    cmbStabilization = new QComboBox(grpFilters);
    cmbStabilization->addItem(tr("Off"), static_cast<int>(FujinonSX800::StabilizationMode::Off));
    cmbStabilization->addItem(tr("OIS (Optical)"), static_cast<int>(FujinonSX800::StabilizationMode::OIS));
    cmbStabilization->addItem(tr("EIS (Electronic)"), static_cast<int>(FujinonSX800::StabilizationMode::EIS));
    cmbStabilization->addItem(tr("Auto (Hybrid)"), static_cast<int>(FujinonSX800::StabilizationMode::Auto));
    filtersLayout->addWidget(cmbStabilization, 3, 1);

    // WDR
    filtersLayout->addWidget(new QLabel(tr("Hyper-DNR / WDR:"), grpFilters), 4, 0);
    cmbWdr = new QComboBox(grpFilters);
    cmbWdr->addItem(tr("Off"), static_cast<int>(FujinonSX800::WdrMode::Off));
    cmbWdr->addItem(tr("Low"), static_cast<int>(FujinonSX800::WdrMode::Low));
    cmbWdr->addItem(tr("Mid"), static_cast<int>(FujinonSX800::WdrMode::Mid));
    cmbWdr->addItem(tr("High"), static_cast<int>(FujinonSX800::WdrMode::High));
    cmbWdr->addItem(tr("Auto"), static_cast<int>(FujinonSX800::WdrMode::Auto));
    filtersLayout->addWidget(cmbWdr, 4, 1);

    // Noise Reduction
    filtersLayout->addWidget(new QLabel(tr("3D Noise Reduction:"), grpFilters), 5, 0);
    cmbNoiseReduction = new QComboBox(grpFilters);
    cmbNoiseReduction->addItem(tr("Off"), static_cast<int>(FujinonSX800::NoiseReduction::Off));
    for (int i { 1 }; i <= 5; ++i) {
        cmbNoiseReduction->addItem(tr("Level %1").arg(i), i);
    }
    filtersLayout->addWidget(cmbNoiseReduction, 5, 1);

    mainLayout->addWidget(grpFilters);

    // ==================== 3. WHITE BALANCE ====================
    auto* grpWb = new QGroupBox(tr("White Balance & Color Calibration"), this);
    auto* wbLayout = new QGridLayout(grpWb);
    wbLayout->setSpacing(10);

    wbLayout->addWidget(new QLabel(tr("Mode:"), grpWb), 0, 0);
    cmbWbMode = new QComboBox(grpWb);
    cmbWbMode->addItem(tr("Auto"), static_cast<int>(FujinonSX800::WhiteBalanceMode::Auto));
    cmbWbMode->addItem(tr("Outdoor"), static_cast<int>(FujinonSX800::WhiteBalanceMode::Outdoor));
    cmbWbMode->addItem(tr("Indoor"), static_cast<int>(FujinonSX800::WhiteBalanceMode::Indoor));
    cmbWbMode->addItem(tr("1-Push Trigger"), static_cast<int>(FujinonSX800::WhiteBalanceMode::OnePush));
    cmbWbMode->addItem(tr("Color Temp"), static_cast<int>(FujinonSX800::WhiteBalanceMode::ColorTemp));
    cmbWbMode->addItem(tr("Manual"), static_cast<int>(FujinonSX800::WhiteBalanceMode::Manual));
    wbLayout->addWidget(cmbWbMode, 0, 1);

    wbLayout->addWidget(new QLabel(tr("Color Temp:"), grpWb), 1, 0);
    sliderColorTemp = new QSlider(Qt::Horizontal, grpWb);
    sliderColorTemp->setRange(2500, 10000);
    sliderColorTemp->setValue(5500);
    wbLayout->addWidget(sliderColorTemp, 1, 1);

    lblColorTempVal = new QLabel("5500 K", grpWb);
    lblColorTempVal->setObjectName("lblTelemetry");
    wbLayout->addWidget(lblColorTempVal, 1, 2);

    wbLayout->addWidget(new QLabel(tr("Red Shift:"), grpWb), 2, 0);
    sliderWbShiftRed = new QSlider(Qt::Horizontal, grpWb);
    sliderWbShiftRed->setRange(-128, 127);
    sliderWbShiftRed->setValue(0);
    wbLayout->addWidget(sliderWbShiftRed, 2, 1);

    lblWbShiftRedVal = new QLabel("0", grpWb);
    lblWbShiftRedVal->setObjectName("lblTelemetry");
    wbLayout->addWidget(lblWbShiftRedVal, 2, 2);

    wbLayout->addWidget(new QLabel(tr("Blue Shift:"), grpWb), 3, 0);
    sliderWbShiftBlue = new QSlider(Qt::Horizontal, grpWb);
    sliderWbShiftBlue->setRange(-128, 127);
    sliderWbShiftBlue->setValue(0);
    wbLayout->addWidget(sliderWbShiftBlue, 3, 1);

    lblWbShiftBlueVal = new QLabel("0", grpWb);
    lblWbShiftBlueVal->setObjectName("lblTelemetry");
    wbLayout->addWidget(lblWbShiftBlueVal, 3, 2);

    mainLayout->addWidget(grpWb);
}

void ImageQualityTab::setupConnections()
{
    // Brightness
    connect(sliderBrightness, &QSlider::sliderReleased, this, [this]() {
        const int val = sliderBrightness->value();
        lblBrightnessVal->setText(QString("%1 (%2)").arg(val).arg(sliderBrightnessFine->value()));
        cam->setBrightness(val);
    });
    connect(sliderBrightnessFine, &QSlider::sliderReleased, this, [this]() {
        const int fine = sliderBrightnessFine->value();
        lblBrightnessVal->setText(QString("%1 (%2)").arg(sliderBrightness->value()).arg(fine));
        cam->setBrightnessFine(fine);
    });

    // Contrast
    connect(sliderContrast, &QSlider::sliderReleased, this, [this]() {
        const int val = sliderContrast->value();
        lblContrastVal->setText(QString("%1 (%2)").arg(val).arg(sliderContrastFine->value()));
        cam->setContrast(val);
    });
    connect(sliderContrastFine, &QSlider::sliderReleased, this, [this]() {
        const int fine = sliderContrastFine->value();
        lblContrastVal->setText(QString("%1 (%2)").arg(sliderContrast->value()).arg(fine));
        cam->setContrastFine(fine);
    });

    // Saturation
    connect(sliderSaturation, &QSlider::sliderReleased, this, [this]() {
        const int val = sliderSaturation->value();
        lblSaturationVal->setText(QString("%1 (%2)").arg(val).arg(sliderSaturationFine->value()));
        cam->setSaturation(val);
    });
    connect(sliderSaturationFine, &QSlider::sliderReleased, this, [this]() {
        const int fine = sliderSaturationFine->value();
        lblSaturationVal->setText(QString("%1 (%2)").arg(sliderSaturation->value()).arg(fine));
        cam->setSaturationFine(fine);
    });

    // Sharpness
    connect(sliderSharpness, &QSlider::sliderReleased, this, [this]() {
        const int val = sliderSharpness->value();
        lblSharpnessVal->setText(QString("%1 (%2)").arg(val).arg(sliderSharpnessFine->value()));
        cam->setSharpness(val);
    });
    connect(sliderSharpnessFine, &QSlider::sliderReleased, this, [this]() {
        const int fine = sliderSharpnessFine->value();
        lblSharpnessVal->setText(QString("%1 (%2)").arg(sliderSharpness->value()).arg(fine));
        cam->setSharpnessFine(fine);
    });

    // Filters
    connect(cmbVlcFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::VlcFilterMode>(cmbVlcFilter->currentData().toInt());
        cam->setVlcFilter(mode);
    });
    connect(cmbDefog, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::DefogMode>(cmbDefog->currentData().toInt());
        cam->setDefog(mode);
    });
    connect(cmbDeHeatHaze, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::DeHeatHazeMode>(cmbDeHeatHaze->currentData().toInt());
        cam->setDeHeatHaze(mode);
    });
    connect(cmbStabilization, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::StabilizationMode>(cmbStabilization->currentData().toInt());
        cam->setStabilization(mode);
    });
    connect(cmbWdr, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::WdrMode>(cmbWdr->currentData().toInt());
        cam->setWdr(mode);
    });
    connect(cmbNoiseReduction, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto nr = static_cast<FujinonSX800::NoiseReduction>(cmbNoiseReduction->currentData().toInt());
        cam->setNoiseReduction(nr);
    });

    // White balance
    connect(cmbWbMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::WhiteBalanceMode>(cmbWbMode->currentData().toInt());
        cam->setWhiteBalance(mode);
    });
    connect(sliderColorTemp, &QSlider::sliderReleased, this, [this]() {
        const int kelvin = sliderColorTemp->value();
        lblColorTempVal->setText(QString("%1 K").arg(kelvin));
        cam->setWbColorTemp(kelvin);
    });
    connect(sliderWbShiftRed, &QSlider::sliderReleased, this, [this]() {
        const int shift = sliderWbShiftRed->value();
        lblWbShiftRedVal->setText(QString::number(shift));
        cam->setWbShiftRedFine(shift);
    });
    connect(sliderWbShiftBlue, &QSlider::sliderReleased, this, [this]() {
        const int shift = sliderWbShiftBlue->value();
        lblWbShiftBlueVal->setText(QString::number(shift));
        cam->setWbShiftBlueFine(shift);
    });
}

void ImageQualityTab::updateTelemetry(const FujinonSX800::CameraStatus& status)
{
    isUpdatingFromTelemetry = true;

    if (!sliderBrightness->isSliderDown()) {
        sliderBrightness->setValue(status.brightness);
    }
    if (!sliderBrightnessFine->isSliderDown()) {
        sliderBrightnessFine->setValue(status.brightnessFine);
    }
    lblBrightnessVal->setText(QString("%1 (%2)").arg(status.brightness).arg(status.brightnessFine));

    if (!sliderContrast->isSliderDown()) {
        sliderContrast->setValue(status.contrast);
    }
    if (!sliderContrastFine->isSliderDown()) {
        sliderContrastFine->setValue(status.contrastFine);
    }
    lblContrastVal->setText(QString("%1 (%2)").arg(status.contrast).arg(status.contrastFine));

    if (!sliderSaturation->isSliderDown()) {
        sliderSaturation->setValue(status.saturation);
    }
    if (!sliderSaturationFine->isSliderDown()) {
        sliderSaturationFine->setValue(status.saturationFine);
    }
    lblSaturationVal->setText(QString("%1 (%2)").arg(status.saturation).arg(status.saturationFine));

    if (!sliderSharpness->isSliderDown()) {
        sliderSharpness->setValue(status.sharpness);
    }
    if (!sliderSharpnessFine->isSliderDown()) {
        sliderSharpnessFine->setValue(status.sharpnessFine);
    }
    lblSharpnessVal->setText(QString("%1 (%2)").arg(status.sharpness).arg(status.sharpnessFine));

    isUpdatingFromTelemetry = false;
}

} // namespace FujinonSX800App

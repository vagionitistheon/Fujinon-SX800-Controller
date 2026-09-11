/// @file OpticsTab.cpp
/// @brief Implementation of lens and exposure controls tab.

#include "OpticsTab.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace FujinonSX800App {

OpticsTab::OpticsTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent)
    : QWidget(parent)
    , cam { camera }
{
    setupUi();
    setupConnections();
}

void OpticsTab::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ==================== 1. ZOOM GROUP ====================
    auto* grpZoom = new QGroupBox(tr("Optical & Digital Zoom"), this);
    auto* zoomLayout = new QVBoxLayout(grpZoom);
    zoomLayout->setSpacing(10);

    // Continuous Buttons
    auto* zoomBtnLayout = new QHBoxLayout();
    btnZoomWide = new QPushButton(tr("<< Zoom Wide"), grpZoom);
    btnZoomTele = new QPushButton(tr("Zoom Tele >>"), grpZoom);
    btnZoomWide->setMinimumHeight(34);
    btnZoomTele->setMinimumHeight(34);
    zoomBtnLayout->addWidget(btnZoomWide);
    zoomBtnLayout->addWidget(btnZoomTele);
    zoomLayout->addLayout(zoomBtnLayout);

    // Zoom Position Slider & SpinBox
    auto* zoomPosLayout = new QGridLayout();
    auto* lblZoomPos = new QLabel(tr("Absolute Position:"), grpZoom);
    sliderZoomPos = new QSlider(Qt::Horizontal, grpZoom);
    sliderZoomPos->setRange(0, 65535);
    spinZoomPos = new QSpinBox(grpZoom);
    spinZoomPos->setRange(0, 65535);

    zoomPosLayout->addWidget(lblZoomPos, 0, 0);
    zoomPosLayout->addWidget(sliderZoomPos, 0, 1);
    zoomPosLayout->addWidget(spinZoomPos, 0, 2);
    zoomLayout->addLayout(zoomPosLayout);

    // Optical Readout
    auto* zoomTelemetryLayout = new QHBoxLayout();
    auto* lblFlTitle = new QLabel(tr("Focal Length:"), grpZoom);
    lblFocalLength = new QLabel("20.0 mm", grpZoom);
    lblFocalLength->setObjectName("lblTelemetry");

    auto* lblHfovTitle = new QLabel(tr("HFOV:"), grpZoom);
    lblHfov = new QLabel("20.97°", grpZoom);
    lblHfov->setObjectName("lblTelemetry");

    zoomTelemetryLayout->addWidget(lblFlTitle);
    zoomTelemetryLayout->addWidget(lblFocalLength);
    zoomTelemetryLayout->addSpacing(16);
    zoomTelemetryLayout->addWidget(lblHfovTitle);
    zoomTelemetryLayout->addWidget(lblHfov);
    zoomTelemetryLayout->addStretch();
    zoomLayout->addLayout(zoomTelemetryLayout);

    // Zoom Speed
    auto* zoomSpeedLayout = new QHBoxLayout();
    auto* lblZoomSpd = new QLabel(tr("Speed (1..10):"), grpZoom);
    sliderZoomSpeed = new QSlider(Qt::Horizontal, grpZoom);
    sliderZoomSpeed->setRange(1, 10);
    sliderZoomSpeed->setValue(5);
    lblZoomSpeedVal = new QLabel("5", grpZoom);
    lblZoomSpeedVal->setFixedWidth(20);

    zoomSpeedLayout->addWidget(lblZoomSpd);
    zoomSpeedLayout->addWidget(sliderZoomSpeed);
    zoomSpeedLayout->addWidget(lblZoomSpeedVal);
    zoomLayout->addLayout(zoomSpeedLayout);

    // Digital Zoom
    auto* dzLayout = new QHBoxLayout();
    auto* lblDzMode = new QLabel(tr("Digital Zoom:"), grpZoom);
    cmbDigitalZoom = new QComboBox(grpZoom);
    cmbDigitalZoom->addItem(tr("Off"), static_cast<int>(FujinonSX800::DigitalZoomMode::Off));
    cmbDigitalZoom->addItem(tr("Combined"), static_cast<int>(FujinonSX800::DigitalZoomMode::Combined));
    cmbDigitalZoom->addItem(tr("Separate"), static_cast<int>(FujinonSX800::DigitalZoomMode::Separate));

    auto* lblDzStep = new QLabel(tr("Step:"), grpZoom);
    cmbDigitalZoomStep = new QComboBox(grpZoom);
    cmbDigitalZoomStep->addItem("1.00x", static_cast<int>(FujinonSX800::DigitalZoomStep::X1_00));
    cmbDigitalZoomStep->addItem("1.25x", static_cast<int>(FujinonSX800::DigitalZoomStep::X1_25));
    cmbDigitalZoomStep->addItem("1.50x", static_cast<int>(FujinonSX800::DigitalZoomStep::X1_50));
    cmbDigitalZoomStep->addItem("1.75x", static_cast<int>(FujinonSX800::DigitalZoomStep::X1_75));
    cmbDigitalZoomStep->addItem("2.00x", static_cast<int>(FujinonSX800::DigitalZoomStep::X2_00));

    dzLayout->addWidget(lblDzMode);
    dzLayout->addWidget(cmbDigitalZoom);
    dzLayout->addWidget(lblDzStep);
    dzLayout->addWidget(cmbDigitalZoomStep);
    zoomLayout->addLayout(dzLayout);
    zoomLayout->addStretch();

    mainLayout->addWidget(grpZoom);

    // ==================== 2. FOCUS GROUP ====================
    auto* grpFocus = new QGroupBox(tr("Focus Mechanics"), this);
    auto* focusLayout = new QVBoxLayout(grpFocus);
    focusLayout->setSpacing(10);

    // Near / Far Buttons & 1-Push AF
    auto* focusBtnLayout = new QHBoxLayout();
    btnFocusNear = new QPushButton(tr("<< Focus Near"), grpFocus);
    btnFocusFar = new QPushButton(tr("Focus Far >>"), grpFocus);
    btnPushToFocus = new QPushButton(tr("1-Push AF"), grpFocus);
    btnFocusNear->setMinimumHeight(34);
    btnFocusFar->setMinimumHeight(34);
    btnPushToFocus->setMinimumHeight(34);
    btnPushToFocus->setObjectName("btnPrimary");

    focusBtnLayout->addWidget(btnFocusNear);
    focusBtnLayout->addWidget(btnFocusFar);
    focusBtnLayout->addWidget(btnPushToFocus);
    focusLayout->addLayout(focusBtnLayout);

    // Focus Position Slider & SpinBox
    auto* focusPosLayout = new QGridLayout();
    auto* lblFocusPos = new QLabel(tr("Absolute Position:"), grpFocus);
    sliderFocusPos = new QSlider(Qt::Horizontal, grpFocus);
    sliderFocusPos->setRange(0, 65535);
    spinFocusPos = new QSpinBox(grpFocus);
    spinFocusPos->setRange(0, 65535);

    focusPosLayout->addWidget(lblFocusPos, 0, 0);
    focusPosLayout->addWidget(sliderFocusPos, 0, 1);
    focusPosLayout->addWidget(spinFocusPos, 0, 2);
    focusLayout->addLayout(focusPosLayout);

    // Estimated Distance Readout
    auto* focusTelemetryLayout = new QHBoxLayout();
    auto* lblDistTitle = new QLabel(tr("Estimated Distance:"), grpFocus);
    lblDistance = new QLabel("10.0 m", grpFocus);
    lblDistance->setObjectName("lblTelemetry");

    chkAutoFocus = new QCheckBox(tr("Continuous Auto Focus"), grpFocus);

    focusTelemetryLayout->addWidget(lblDistTitle);
    focusTelemetryLayout->addWidget(lblDistance);
    focusTelemetryLayout->addSpacing(16);
    focusTelemetryLayout->addWidget(chkAutoFocus);
    focusTelemetryLayout->addStretch();
    focusLayout->addLayout(focusTelemetryLayout);

    // Focus Speed
    auto* focusSpeedLayout = new QHBoxLayout();
    auto* lblFocSpd = new QLabel(tr("Speed (1..10):"), grpFocus);
    sliderFocusSpeed = new QSlider(Qt::Horizontal, grpFocus);
    sliderFocusSpeed->setRange(1, 10);
    sliderFocusSpeed->setValue(5);
    lblFocusSpeedVal = new QLabel("5", grpFocus);
    lblFocusSpeedVal->setFixedWidth(20);

    focusSpeedLayout->addWidget(lblFocSpd);
    focusSpeedLayout->addWidget(sliderFocusSpeed);
    focusSpeedLayout->addWidget(lblFocusSpeedVal);
    focusLayout->addLayout(focusSpeedLayout);

    // AF Area & Sensitivity
    auto* afAreaLayout = new QHBoxLayout();
    auto* lblAfArea = new QLabel(tr("AF Area:"), grpFocus);
    cmbAfArea = new QComboBox(grpFocus);
    cmbAfArea->addItem(tr("Center"), static_cast<int>(FujinonSX800::AfArea::Center));
    cmbAfArea->addItem(tr("Full Area"), static_cast<int>(FujinonSX800::AfArea::Full));
    for (int i { 1 }; i <= 9; ++i) {
        cmbAfArea->addItem(tr("Zone %1").arg(i), i);
    }

    auto* lblAfSens = new QLabel(tr("Sensitivity:"), grpFocus);
    cmbAfSensitivity = new QComboBox(grpFocus);
    cmbAfSensitivity->addItem(tr("Low"), static_cast<int>(FujinonSX800::AfSensitivity::Low));
    cmbAfSensitivity->addItem(tr("Normal"), static_cast<int>(FujinonSX800::AfSensitivity::Normal));
    cmbAfSensitivity->addItem(tr("High"), static_cast<int>(FujinonSX800::AfSensitivity::High));

    afAreaLayout->addWidget(lblAfArea);
    afAreaLayout->addWidget(cmbAfArea);
    afAreaLayout->addWidget(lblAfSens);
    afAreaLayout->addWidget(cmbAfSensitivity);
    focusLayout->addLayout(afAreaLayout);
    focusLayout->addStretch();

    mainLayout->addWidget(grpFocus);

    // ==================== 3. IRIS & EXPOSURE ====================
    auto* grpIris = new QGroupBox(tr("Iris & Exposure"), this);
    auto* irisLayout = new QVBoxLayout(grpIris);
    irisLayout->setSpacing(10);

    chkAutoIris = new QCheckBox(tr("Auto Iris"), grpIris);
    chkAutoIris->setChecked(true);
    irisLayout->addWidget(chkAutoIris);

    auto* irisPosLayout = new QHBoxLayout();
    auto* lblIris = new QLabel(tr("Manual Iris:"), grpIris);
    sliderIris = new QSlider(Qt::Horizontal, grpIris);
    sliderIris->setRange(0, 1023);
    sliderIris->setValue(512);
    sliderIris->setEnabled(false);

    cmbManualIrisFNo = new QComboBox(grpIris);
    cmbManualIrisFNo->addItem(tr("Close"), static_cast<int>(FujinonSX800::ManualIrisFNo::Close));
    cmbManualIrisFNo->addItem("F4.0", static_cast<int>(FujinonSX800::ManualIrisFNo::F4_0));
    cmbManualIrisFNo->addItem("F4.5", static_cast<int>(FujinonSX800::ManualIrisFNo::F4_5));
    cmbManualIrisFNo->addItem("F5.0", static_cast<int>(FujinonSX800::ManualIrisFNo::F5_0));
    cmbManualIrisFNo->addItem("F5.6", static_cast<int>(FujinonSX800::ManualIrisFNo::F5_6));
    cmbManualIrisFNo->addItem("F8.0", static_cast<int>(FujinonSX800::ManualIrisFNo::F8_0));
    cmbManualIrisFNo->addItem("F11", static_cast<int>(FujinonSX800::ManualIrisFNo::F11));
    cmbManualIrisFNo->addItem("F16", static_cast<int>(FujinonSX800::ManualIrisFNo::F16));
    cmbManualIrisFNo->addItem("F22", static_cast<int>(FujinonSX800::ManualIrisFNo::F22));
    cmbManualIrisFNo->setEnabled(false);

    irisPosLayout->addWidget(lblIris);
    irisPosLayout->addWidget(sliderIris);
    irisPosLayout->addWidget(cmbManualIrisFNo);
    irisLayout->addLayout(irisPosLayout);

    auto* exposureCheckLayout = new QHBoxLayout();
    chkAgc = new QCheckBox(tr("Auto Gain (AGC)"), grpIris);
    chkAgc->setChecked(true);
    chkBlc = new QCheckBox(tr("Backlight Comp (BLC)"), grpIris);
    exposureCheckLayout->addWidget(chkAgc);
    exposureCheckLayout->addWidget(chkBlc);
    irisLayout->addLayout(exposureCheckLayout);

    auto* shutterLimitLayout = new QHBoxLayout();
    auto* lblShutterLimit = new QLabel(tr("Auto Shutter Limit:"), grpIris);
    cmbShutterLimit = new QComboBox(grpIris);
    cmbShutterLimit->addItem(tr("Manual / Off"), static_cast<int>(FujinonSX800::ShutterLimitMode::Manual));
    cmbShutterLimit->addItem("1/8 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_8));
    cmbShutterLimit->addItem("1/15 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_15));
    cmbShutterLimit->addItem("1/30 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_30));
    cmbShutterLimit->addItem("1/60 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_60));
    cmbShutterLimit->addItem("1/125 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_125));
    cmbShutterLimit->addItem("1/250 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_250));
    cmbShutterLimit->addItem("1/500 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_500));
    cmbShutterLimit->addItem("1/1000 s", static_cast<int>(FujinonSX800::ShutterLimitMode::Lowest1_1000));
    shutterLimitLayout->addWidget(lblShutterLimit);
    shutterLimitLayout->addWidget(cmbShutterLimit);
    irisLayout->addLayout(shutterLimitLayout);

    auto* shutterLayout = new QHBoxLayout();
    auto* lblShutter = new QLabel(tr("Manual Shutter:"), grpIris);
    spinShutter = new QSpinBox(grpIris);
    spinShutter->setRange(1, 10000);
    spinShutter->setValue(100);
    spinShutter->setSuffix(" 1/s");

    shutterLayout->addWidget(lblShutter);
    shutterLayout->addWidget(spinShutter);
    irisLayout->addLayout(shutterLayout);

    auto* isoLayout = new QHBoxLayout();
    auto* lblIso = new QLabel(tr("ISO Gain:"), grpIris);
    spinIso = new QSpinBox(grpIris);
    spinIso->setRange(100, 25600);
    spinIso->setValue(400);

    isoLayout->addWidget(lblIso);
    isoLayout->addWidget(spinIso);
    irisLayout->addLayout(isoLayout);

    irisLayout->addStretch();
    mainLayout->addWidget(grpIris);
}

void OpticsTab::setupConnections()
{
    // Zoom Motion Press/Release
    connect(btnZoomTele, &QPushButton::pressed, cam, &FujinonSX800Qt::QFujinonCamera::zoomTele);
    connect(btnZoomTele, &QPushButton::released, cam, &FujinonSX800Qt::QFujinonCamera::zoomStop);
    connect(btnZoomWide, &QPushButton::pressed, cam, &FujinonSX800Qt::QFujinonCamera::zoomWide);
    connect(btnZoomWide, &QPushButton::released, cam, &FujinonSX800Qt::QFujinonCamera::zoomStop);

    // Zoom Position Sliders
    connect(sliderZoomPos, &QSlider::valueChanged, spinZoomPos, &QSpinBox::setValue);
    connect(spinZoomPos, QOverload<int>::of(&QSpinBox::valueChanged), sliderZoomPos, &QSlider::setValue);
    connect(sliderZoomPos, &QSlider::sliderReleased, this, [this]() {
        if (!isUpdatingFromTelemetry) {
            cam->setZoomPosition(sliderZoomPos->value());
        }
    });

    // Zoom Speed
    connect(sliderZoomSpeed, &QSlider::valueChanged, this, [this](int val) {
        lblZoomSpeedVal->setText(QString::number(val));
        cam->setZoomSpeed(val);
    });

    // Digital Zoom
    connect(cmbDigitalZoom, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::DigitalZoomMode>(cmbDigitalZoom->currentData().toInt());
        cam->setDigitalZoom(mode);
    });
    connect(cmbDigitalZoomStep, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto step = static_cast<FujinonSX800::DigitalZoomStep>(cmbDigitalZoomStep->currentData().toInt());
        cam->setDigitalZoomStep(step);
    });

    // Focus Motion Press/Release
    connect(btnFocusNear, &QPushButton::pressed, cam, &FujinonSX800Qt::QFujinonCamera::focusNear);
    connect(btnFocusNear, &QPushButton::released, cam, &FujinonSX800Qt::QFujinonCamera::focusStop);
    connect(btnFocusFar, &QPushButton::pressed, cam, &FujinonSX800Qt::QFujinonCamera::focusFar);
    connect(btnFocusFar, &QPushButton::released, cam, &FujinonSX800Qt::QFujinonCamera::focusStop);

    // 1-Push AF and Continuous AF
    connect(btnPushToFocus, &QPushButton::clicked, cam, &FujinonSX800Qt::QFujinonCamera::triggerOnePushAf);
    connect(chkAutoFocus, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setAutoFocus);

    // Focus Position Sliders
    connect(sliderFocusPos, &QSlider::valueChanged, spinFocusPos, &QSpinBox::setValue);
    connect(spinFocusPos, QOverload<int>::of(&QSpinBox::valueChanged), sliderFocusPos, &QSlider::setValue);
    connect(sliderFocusPos, &QSlider::sliderReleased, this, [this]() {
        if (!isUpdatingFromTelemetry) {
            cam->setFocusPosition(sliderFocusPos->value());
        }
    });

    // Focus Speed
    connect(sliderFocusSpeed, &QSlider::valueChanged, this, [this](int val) {
        lblFocusSpeedVal->setText(QString::number(val));
        cam->setFocusSpeed(val);
    });

    // AF Area & Sensitivity
    connect(cmbAfArea, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto area = static_cast<FujinonSX800::AfArea>(cmbAfArea->currentData().toInt());
        cam->setAfArea(area);
    });
    connect(cmbAfSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto sens = static_cast<FujinonSX800::AfSensitivity>(cmbAfSensitivity->currentData().toInt());
        cam->setAfSensitivity(sens);
    });

    // Iris & Exposure
    connect(chkAutoIris, &QCheckBox::toggled, this, [this](bool checked) {
        sliderIris->setEnabled(!checked);
        cmbManualIrisFNo->setEnabled(!checked);
        cam->setAutoIris(checked);
    });
    connect(sliderIris, &QSlider::sliderReleased, this, [this]() {
        const auto fNo = FujinonSX800::positionToManualIrisFNo(static_cast<std::uint16_t>(sliderIris->value()));
        cam->setManualIrisFNo(fNo);
        const int idx = cmbManualIrisFNo->findData(static_cast<int>(fNo));
        if (idx >= 0) {
            cmbManualIrisFNo->setCurrentIndex(idx);
        }
    });
    connect(cmbManualIrisFNo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (!isUpdatingFromTelemetry) {
            const auto fNo = static_cast<FujinonSX800::ManualIrisFNo>(cmbManualIrisFNo->currentData().toInt());
            cam->setManualIrisFNo(fNo);
        }
    });
    connect(cmbShutterLimit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto limit = static_cast<FujinonSX800::ShutterLimitMode>(cmbShutterLimit->currentData().toInt());
        cam->setShutterLimit(limit);
    });
    connect(chkAgc, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setAgc);
    connect(chkBlc, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setBlc);
    connect(spinShutter, QOverload<int>::of(&QSpinBox::valueChanged), cam,
        &FujinonSX800Qt::QFujinonCamera::setManualShutter);
    connect(spinIso, QOverload<int>::of(&QSpinBox::valueChanged), cam, &FujinonSX800Qt::QFujinonCamera::setManualIso);
}

void OpticsTab::updateTelemetry(const FujinonSX800::CameraStatus& status)
{
    isUpdatingFromTelemetry = true;

    // Zoom readouts
    if (!sliderZoomPos->isSliderDown()) {
        sliderZoomPos->setValue(status.zoomPosition);
    }
    lblFocalLength->setText(QString("%1 mm").arg(status.focalLengthMm, 0, 'f', 1));
    lblHfov->setText(QString("%1°").arg(status.horizontalFovDeg, 0, 'f', 2));

    // Focus readouts
    if (!sliderFocusPos->isSliderDown()) {
        sliderFocusPos->setValue(status.focusPosition);
    }
    lblDistance->setText(QString("%1 m").arg(status.focusDistanceM, 0, 'f', 1));

    chkAutoFocus->setChecked(status.autoFocusMode == FujinonSX800::AutoFocusMode::On);
    chkAutoIris->setChecked(status.autoIrisMode == FujinonSX800::AutoIrisMode::On);
    if (!sliderIris->isSliderDown()) {
        const int idx = cmbManualIrisFNo->findData(static_cast<int>(status.manualIrisFNo));
        if (idx >= 0) {
            cmbManualIrisFNo->setCurrentIndex(idx);
        }
    }
    chkAgc->setChecked(status.agcMode == FujinonSX800::AgcMode::Auto);
    chkBlc->setChecked(status.blcMode == FujinonSX800::BlcMode::On);

    isUpdatingFromTelemetry = false;
}

} // namespace FujinonSX800App

/// @file SystemTab.cpp
/// @brief Implementation of hardware diagnostics and maintenance tab.

#include "SystemTab.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>

namespace FujinonSX800App {

SystemTab::SystemTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent)
    : QWidget(parent)
    , cam { camera }
{
    setupUi();
    setupConnections();
}

void SystemTab::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ==================== 1. HARDWARE TELEMETRY ====================
    auto* grpTelemetry = new QGroupBox(tr("Camera Telemetry & Identification"), this);
    auto* telemLayout = new QGridLayout(grpTelemetry);
    telemLayout->setSpacing(10);

    telemLayout->addWidget(new QLabel(tr("Serial Number:"), grpTelemetry), 0, 0);
    lblSerialVal = new QLabel("N/A", grpTelemetry);
    lblSerialVal->setObjectName("lblTelemetry");
    telemLayout->addWidget(lblSerialVal, 0, 1);

    telemLayout->addWidget(new QLabel(tr("Firmware Version:"), grpTelemetry), 1, 0);
    lblFwVal = new QLabel("N/A", grpTelemetry);
    lblFwVal->setObjectName("lblTelemetry");
    telemLayout->addWidget(lblFwVal, 1, 1);

    telemLayout->addWidget(new QLabel(tr("Device Address:"), grpTelemetry), 2, 0);
    lblAddressVal = new QLabel("7", grpTelemetry);
    lblAddressVal->setObjectName("lblTelemetry");
    telemLayout->addWidget(lblAddressVal, 2, 1);

    telemLayout->addWidget(new QLabel(tr("Lens Temperature:"), grpTelemetry), 3, 0);
    lblTempVal = new QLabel("25.0 °C", grpTelemetry);
    lblTempVal->setObjectName("lblTelemetry");
    telemLayout->addWidget(lblTempVal, 3, 1);

    mainLayout->addWidget(grpTelemetry);

    // ==================== 2. LENS STATUS DIAGNOSTICS ====================
    auto* grpLens = new QGroupBox(tr("Lens Mechanism Diagnostics"), this);
    auto* lensLayout = new QGridLayout(grpLens);
    lensLayout->setSpacing(10);

    lensLayout->addWidget(new QLabel(tr("Zoom Motion:"), grpLens), 0, 0);
    lblZoomMoving = new QLabel(tr("Idle"), grpLens);
    lblZoomMoving->setObjectName("lblTelemetry");
    lensLayout->addWidget(lblZoomMoving, 0, 1);

    lensLayout->addWidget(new QLabel(tr("Zoom Limits:"), grpLens), 1, 0);
    lblZoomLimits = new QLabel(tr("Normal"), grpLens);
    lblZoomLimits->setObjectName("lblTelemetry");
    lensLayout->addWidget(lblZoomLimits, 1, 1);

    lensLayout->addWidget(new QLabel(tr("Focus Motion:"), grpLens), 2, 0);
    lblFocusMoving = new QLabel(tr("Idle"), grpLens);
    lblFocusMoving->setObjectName("lblTelemetry");
    lensLayout->addWidget(lblFocusMoving, 2, 1);

    lensLayout->addWidget(new QLabel(tr("Focus Limits:"), grpLens), 3, 0);
    lblFocusLimits = new QLabel(tr("Normal"), grpLens);
    lblFocusLimits->setObjectName("lblTelemetry");
    lensLayout->addWidget(lblFocusLimits, 3, 1);

    lensLayout->addWidget(new QLabel(tr("Iris Motion:"), grpLens), 4, 0);
    lblIrisMoving = new QLabel(tr("Idle"), grpLens);
    lblIrisMoving->setObjectName("lblTelemetry");
    lensLayout->addWidget(lblIrisMoving, 4, 1);

    lensLayout->addWidget(new QLabel(tr("Iris Limits:"), grpLens), 5, 0);
    lblIrisLimits = new QLabel(tr("Normal"), grpLens);
    lblIrisLimits->setObjectName("lblTelemetry");
    lensLayout->addWidget(lblIrisLimits, 5, 1);

    mainLayout->addWidget(grpLens);

    // ==================== 3. SYSTEM MAINTENANCE ====================
    auto* grpMaint = new QGroupBox(tr("System Settings & Maintenance"), this);
    auto* maintLayout = new QVBoxLayout(grpMaint);
    maintLayout->setSpacing(10);

    chkTermination = new QCheckBox(tr("RS-485 120Ω Internal Termination"), grpMaint);
    maintLayout->addWidget(chkTermination);

    auto* langLayout = new QHBoxLayout();
    langLayout->addWidget(new QLabel(tr("OSD Language:"), grpMaint));
    cmbLanguage = new QComboBox(grpMaint);
    cmbLanguage->addItem(tr("English"), static_cast<int>(FujinonSX800::Language::English));
    cmbLanguage->addItem(tr("Japanese"), static_cast<int>(FujinonSX800::Language::Japanese));
    cmbLanguage->addItem(tr("Chinese"), static_cast<int>(FujinonSX800::Language::Chinese));
    langLayout->addWidget(cmbLanguage);
    maintLayout->addLayout(langLayout);

    btnRefreshAll = new QPushButton(tr("Query All Telemetry"), grpMaint);
    btnRefreshAll->setObjectName("btnPrimary");
    maintLayout->addWidget(btnRefreshAll);

    auto* pollLayout = new QHBoxLayout();
    chkContinuousPolling = new QCheckBox(tr("Continuous Polling:"), grpMaint);
    pollLayout->addWidget(chkContinuousPolling);

    spinPollInterval = new QSpinBox(grpMaint);
    spinPollInterval->setRange(200, 10000);
    spinPollInterval->setValue(1000);
    spinPollInterval->setSingleStep(100);
    spinPollInterval->setSuffix(" ms");
    pollLayout->addWidget(spinPollInterval);
    maintLayout->addLayout(pollLayout);

    btnResetDefaults = new QPushButton(tr("Factory Reset Defaults..."), grpMaint);
    btnResetDefaults->setObjectName("btnDanger");
    maintLayout->addWidget(btnResetDefaults);

    btnReboot = new QPushButton(tr("Reboot Camera Hardware..."), grpMaint);
    btnReboot->setObjectName("btnDanger");
    maintLayout->addWidget(btnReboot);

    maintLayout->addStretch();
    mainLayout->addWidget(grpMaint);
}

void SystemTab::setupConnections()
{
    connect(chkTermination, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setRs485Termination);

    connect(cmbLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto lang = static_cast<FujinonSX800::Language>(cmbLanguage->currentData().toInt());
        cam->setLanguage(lang);
    });

    connect(btnRefreshAll, &QPushButton::clicked, cam, &FujinonSX800Qt::QFujinonCamera::refreshStatus);

    connect(chkContinuousPolling, &QCheckBox::toggled, this, [this](bool checked) {
        cam->setTelemetryPolling(checked, spinPollInterval->value());
    });

    connect(spinPollInterval, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
        if (chkContinuousPolling->isChecked()) {
            cam->setTelemetryPolling(true, val);
        }
    });

    connect(btnResetDefaults, &QPushButton::clicked, this, [this]() {
        const auto res = QMessageBox::warning(this, tr("Factory Reset"),
            tr("Reset camera parameters to factory defaults?"), QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            cam->resetDefaults(0);
        }
    });

    connect(btnReboot, &QPushButton::clicked, this, [this]() {
        const auto res = QMessageBox::warning(
            this, tr("Reboot Camera"), tr("Reboot camera hardware now?"), QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            cam->rebootCamera();
        }
    });
}

void SystemTab::updateTelemetry(const FujinonSX800::CameraStatus& status)
{
    if (!status.serialNumber.empty()) {
        lblSerialVal->setText(QString::fromStdString(status.serialNumber));
    }
    if (status.fwVersionMajor > 0 || status.fwVersionMinor > 0) {
        lblFwVal->setText(QString("v%1.%2").arg(status.fwVersionMajor).arg(status.fwVersionMinor));
    }
    lblAddressVal->setText(QString::number(status.rs485Address));
    lblTempVal->setText(QString("%1 °C").arg(status.internalTemperatureC, 0, 'f', 1));

    lblZoomMoving->setText(status.zoomMoving ? tr("MOVING") : tr("Idle"));
    lblZoomMoving->setStyleSheet(status.zoomMoving ? "color: #e3b341; font-weight: bold;" : "color: #58a6ff;");

    QString zoomLimitStr = tr("Normal");
    if (status.zoomTeleLimit)
        zoomLimitStr = tr("Tele Limit");
    if (status.zoomWideLimit)
        zoomLimitStr = tr("Wide Limit");
    lblZoomLimits->setText(zoomLimitStr);

    lblFocusMoving->setText(status.focusMoving ? tr("MOVING") : tr("Idle"));
    lblFocusMoving->setStyleSheet(status.focusMoving ? "color: #e3b341; font-weight: bold;" : "color: #58a6ff;");

    QString focusLimitStr = tr("Normal");
    if (status.focusFarLimit)
        focusLimitStr = tr("Far Limit (Infinity)");
    if (status.focusNearLimit)
        focusLimitStr = tr("Near Limit");
    lblFocusLimits->setText(focusLimitStr);

    lblIrisMoving->setText(status.irisMoving ? tr("MOVING") : tr("Idle"));
    lblIrisMoving->setStyleSheet(status.irisMoving ? "color: #e3b341; font-weight: bold;" : "color: #58a6ff;");

    QString irisLimitStr = tr("Normal");
    if (status.irisCloseLimit)
        irisLimitStr = tr("Closed Limit");
    if (status.irisOpenLimit)
        irisLimitStr = tr("Open Limit");
    lblIrisLimits->setText(irisLimitStr);

    chkTermination->setChecked(status.rs485Termination);
}

} // namespace FujinonSX800App

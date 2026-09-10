/// @file DayNightTab.cpp
/// @brief Implementation of Day/Night switching and scheduling tab.

#include "DayNightTab.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace FujinonSX800App {

DayNightTab::DayNightTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent)
    : QWidget(parent)
    , cam { camera }
{
    setupUi();
    setupConnections();
}

void DayNightTab::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ==================== 1. MODE & THRESHOLDS ====================
    auto* grpMode = new QGroupBox(tr("Day / Night Mode & Auto Switching"), this);
    auto* modeLayout = new QGridLayout(grpMode);
    modeLayout->setSpacing(10);

    modeLayout->addWidget(new QLabel(tr("Operation Mode:"), grpMode), 0, 0);
    cmbMode = new QComboBox(grpMode);
    cmbMode->addItem(tr("Auto (Sensor Lux)"), static_cast<int>(FujinonSX800::DayNightMode::Auto));
    cmbMode->addItem(tr("Manual Day"), static_cast<int>(FujinonSX800::DayNightMode::Day));
    cmbMode->addItem(tr("Manual Night"), static_cast<int>(FujinonSX800::DayNightMode::Night));
    cmbMode->addItem(tr("Schedule Timer"), static_cast<int>(FujinonSX800::DayNightMode::Schedule));
    cmbMode->addItem(tr("External Signal"), static_cast<int>(FujinonSX800::DayNightMode::External));
    modeLayout->addWidget(cmbMode, 0, 1);

    modeLayout->addWidget(new QLabel(tr("Day to Night Threshold:"), grpMode), 1, 0);
    spinDayToNightTh = new QSpinBox(grpMode);
    spinDayToNightTh->setRange(0, 100);
    spinDayToNightTh->setValue(30);
    modeLayout->addWidget(spinDayToNightTh, 1, 1);

    modeLayout->addWidget(new QLabel(tr("Night to Day Threshold:"), grpMode), 2, 0);
    spinNightToDayTh = new QSpinBox(grpMode);
    spinNightToDayTh->setRange(0, 100);
    spinNightToDayTh->setValue(50);
    modeLayout->addWidget(spinNightToDayTh, 2, 1);

    modeLayout->addWidget(new QLabel(tr("Switching Delay (sec):"), grpMode), 3, 0);
    spinAutoDelay = new QSpinBox(grpMode);
    spinAutoDelay->setRange(1, 60);
    spinAutoDelay->setValue(5);
    modeLayout->addWidget(spinAutoDelay, 3, 1);

    mainLayout->addWidget(grpMode);

    // ==================== 2. SCHEDULE & OPTICAL FILTERS ====================
    auto* grpSchedule = new QGroupBox(tr("Schedule & Physical Filter Selection"), this);
    auto* scheduleLayout = new QGridLayout(grpSchedule);
    scheduleLayout->setSpacing(10);

    scheduleLayout->addWidget(new QLabel(tr("Day Mode Start Time:"), grpSchedule), 0, 0);
    timeDayStart = new QTimeEdit(QTime(6, 0), grpSchedule);
    scheduleLayout->addWidget(timeDayStart, 0, 1);

    scheduleLayout->addWidget(new QLabel(tr("Night Mode Start Time:"), grpSchedule), 1, 0);
    timeNightStart = new QTimeEdit(QTime(18, 0), grpSchedule);
    scheduleLayout->addWidget(timeNightStart, 1, 1);

    scheduleLayout->addWidget(new QLabel(tr("Optical Filter (Day):"), grpSchedule), 2, 0);
    cmbFilterDay = new QComboBox(grpSchedule);
    cmbFilterDay->addItem(tr("Visible (Standard)"), static_cast<int>(FujinonSX800::OpticalFilter::Visible));
    cmbFilterDay->addItem(tr("VLC Filter"), static_cast<int>(FujinonSX800::OpticalFilter::Vlc));
    cmbFilterDay->addItem(tr("IR Bandpass"), static_cast<int>(FujinonSX800::OpticalFilter::IR));
    scheduleLayout->addWidget(cmbFilterDay, 2, 1);

    scheduleLayout->addWidget(new QLabel(tr("Optical Filter (Night):"), grpSchedule), 3, 0);
    cmbFilterNight = new QComboBox(grpSchedule);
    cmbFilterNight->addItem(tr("IR Bandpass"), static_cast<int>(FujinonSX800::OpticalFilter::IR));
    cmbFilterNight->addItem(tr("Visible"), static_cast<int>(FujinonSX800::OpticalFilter::Visible));
    cmbFilterNight->addItem(tr("VLC Filter"), static_cast<int>(FujinonSX800::OpticalFilter::Vlc));
    scheduleLayout->addWidget(cmbFilterNight, 3, 1);

    scheduleLayout->addWidget(new QLabel(tr("Infrared Wavelength:"), grpSchedule), 4, 0);
    cmbIrWavelength = new QComboBox(grpSchedule);
    cmbIrWavelength->addItem(tr("Standard IR"), static_cast<int>(FujinonSX800::IrWavelength::Standard));
    cmbIrWavelength->addItem(tr("950 nm (Near-IR)"), static_cast<int>(FujinonSX800::IrWavelength::Wave950nm));
    scheduleLayout->addWidget(cmbIrWavelength, 4, 1);

    mainLayout->addWidget(grpSchedule);
}

void DayNightTab::setupConnections()
{
    connect(cmbMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::DayNightMode>(cmbMode->currentData().toInt());
        cam->setDayNightMode(mode);
    });

    connect(spinDayToNightTh, QOverload<int>::of(&QSpinBox::valueChanged), this,
        [this]() { cam->setDayNightThresholds(spinDayToNightTh->value(), spinNightToDayTh->value()); });
    connect(spinNightToDayTh, QOverload<int>::of(&QSpinBox::valueChanged), this,
        [this]() { cam->setDayNightThresholds(spinDayToNightTh->value(), spinNightToDayTh->value()); });

    connect(spinAutoDelay, QOverload<int>::of(&QSpinBox::valueChanged), this,
        [this](int sec) { cam->setDayNightDelay(sec); });

    connect(cmbFilterDay, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto f = static_cast<FujinonSX800::OpticalFilter>(cmbFilterDay->currentData().toInt());
        cam->setOpticalFilterDay(f);
    });
    connect(cmbFilterNight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto f = static_cast<FujinonSX800::OpticalFilter>(cmbFilterNight->currentData().toInt());
        cam->setOpticalFilterNight(f);
    });
    connect(cmbIrWavelength, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto wave = static_cast<FujinonSX800::IrWavelength>(cmbIrWavelength->currentData().toInt());
        cam->setIrWavelength(wave);
    });
}

void DayNightTab::updateTelemetry(const FujinonSX800::CameraStatus& status)
{
    isUpdatingFromTelemetry = true;

    const int modeIdx = cmbMode->findData(static_cast<int>(status.dayNightMode));
    if (modeIdx >= 0) {
        cmbMode->setCurrentIndex(modeIdx);
    }

    spinDayToNightTh->setValue(status.dayToNightThreshold);
    spinNightToDayTh->setValue(status.nightToDayThreshold);
    spinAutoDelay->setValue(status.dayNightAutoDelaySec);

    isUpdatingFromTelemetry = false;
}

} // namespace FujinonSX800App

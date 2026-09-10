#pragma once

/// @file DayNightTab.h
/// @brief UI Tab for Day/Night modes, optical filters, thresholds, and scheduling.

#include "CameraStatus.h"
#include "QFujinonCamera.h"

#include <QComboBox>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QTimeEdit>
#include <QWidget>

namespace FujinonSX800App {

/// @class DayNightTab
/// @brief Manages day/night optical filter switching, schedules, and thresholds.
class DayNightTab : public QWidget {
    Q_OBJECT

public:
    explicit DayNightTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent = nullptr);
    ~DayNightTab() override = default;

public slots:
    void updateTelemetry(const FujinonSX800::CameraStatus& status);

private:
    void setupUi();
    void setupConnections();

    FujinonSX800Qt::QFujinonCamera* cam { nullptr };

    QComboBox* cmbMode { nullptr };
    QSpinBox* spinDayToNightTh { nullptr };
    QSpinBox* spinNightToDayTh { nullptr };
    QSpinBox* spinAutoDelay { nullptr };

    QTimeEdit* timeDayStart { nullptr };
    QTimeEdit* timeNightStart { nullptr };

    QComboBox* cmbFilterDay { nullptr };
    QComboBox* cmbFilterNight { nullptr };
    QComboBox* cmbIrWavelength { nullptr };

    bool isUpdatingFromTelemetry { false };
};

} // namespace FujinonSX800App

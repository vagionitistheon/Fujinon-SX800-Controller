#pragma once

/// @file OsdVideoTab.h
/// @brief UI Tab for On-Screen Display (OSD) and Video Output configuration.

#include "CameraStatus.h"
#include "QFujinonCamera.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

namespace FujinonSX800App {

/// @class OsdVideoTab
/// @brief Manages camera OSD text overlays, reticle, and HDMI/SDI video standards.
class OsdVideoTab : public QWidget {
    Q_OBJECT

public:
    explicit OsdVideoTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent = nullptr);
    ~OsdVideoTab() override = default;

public slots:
    void updateTelemetry(const FujinonSX800::CameraStatus& status);

private:
    void setupUi();
    void setupConnections();

    FujinonSX800Qt::QFujinonCamera* cam { nullptr };

    // OSD Overlays
    QCheckBox* chkDateTimeOsd { nullptr };
    QComboBox* cmbTimeFormat { nullptr };
    QComboBox* cmbDateFormat { nullptr };
    QComboBox* cmbDatePos { nullptr };

    QCheckBox* chkTitleOsd { nullptr };
    QLineEdit* editTitleText { nullptr };
    QComboBox* cmbTitlePos { nullptr };

    QCheckBox* chkIdOsd { nullptr };
    QComboBox* cmbIdPos { nullptr };

    QCheckBox* chkCenterCrosshair { nullptr };
    QCheckBox* chkAntialiasing { nullptr };

    // RTC Sync
    QDateTimeEdit* editRtcTime { nullptr };
    QPushButton* btnSyncRtc { nullptr };

    // Video Format & Display
    QComboBox* cmbVideoStandard { nullptr };
    QComboBox* cmbHdFormat { nullptr };
    QComboBox* cmbVideoDisplayMode { nullptr };

    bool isUpdatingFromTelemetry { false };
};

} // namespace FujinonSX800App

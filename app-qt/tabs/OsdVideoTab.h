#pragma once

/// @file OsdVideoTab.h
/// @brief UI Tab for On-Screen Display (OSD) and Video Output configuration.

#include "CameraStatus.h"
#include "QFujinonCamera.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
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

    QCheckBox* chkTitleOsd { nullptr };
    QLineEdit* editTitleText { nullptr };

    QCheckBox* chkIdOsd { nullptr };
    QCheckBox* chkCenterCrosshair { nullptr };
    QCheckBox* chkAntialiasing { nullptr };

    // Video Format
    QComboBox* cmbVideoStandard { nullptr };
    QComboBox* cmbHdFormat { nullptr };

    bool isUpdatingFromTelemetry { false };
};

} // namespace FujinonSX800App

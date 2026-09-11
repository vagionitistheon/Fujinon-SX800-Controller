/// @file OsdVideoTab.cpp
/// @brief Implementation of OSD and video format tab.

#include "OsdVideoTab.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace FujinonSX800App {

OsdVideoTab::OsdVideoTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent)
    : QWidget(parent)
    , cam { camera }
{
    setupUi();
    setupConnections();
}

void OsdVideoTab::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ==================== 1. OSD OVERLAYS ====================
    auto* grpOsd = new QGroupBox(tr("On-Screen Display (OSD) Overlays"), this);
    auto* osdLayout = new QGridLayout(grpOsd);
    osdLayout->setSpacing(10);

    chkDateTimeOsd = new QCheckBox(tr("Display Date / Time"), grpOsd);
    osdLayout->addWidget(chkDateTimeOsd, 0, 0);

    cmbTimeFormat = new QComboBox(grpOsd);
    cmbTimeFormat->addItem(tr("24-Hour Format"), static_cast<int>(FujinonSX800::TimeDisplayFormat::Hour24));
    cmbTimeFormat->addItem(tr("12-Hour Format"), static_cast<int>(FujinonSX800::TimeDisplayFormat::Hour12));
    osdLayout->addWidget(cmbTimeFormat, 0, 1);

    cmbDateFormat = new QComboBox(grpOsd);
    cmbDateFormat->addItem("YYYY/MM/DD", static_cast<int>(FujinonSX800::DateDisplayFormat::YMD));
    cmbDateFormat->addItem("MM/DD/YYYY", static_cast<int>(FujinonSX800::DateDisplayFormat::MDY));
    cmbDateFormat->addItem("DD/MM/YYYY", static_cast<int>(FujinonSX800::DateDisplayFormat::DMY));
    osdLayout->addWidget(cmbDateFormat, 0, 2);

    cmbDatePos = new QComboBox(grpOsd);
    cmbDatePos->addItem(tr("Top Left"), static_cast<int>(FujinonSX800::OsdPosition::TopLeft));
    cmbDatePos->addItem(tr("Top Right"), static_cast<int>(FujinonSX800::OsdPosition::TopRight));
    cmbDatePos->addItem(tr("Bottom Left"), static_cast<int>(FujinonSX800::OsdPosition::BottomLeft));
    cmbDatePos->addItem(tr("Bottom Right"), static_cast<int>(FujinonSX800::OsdPosition::BottomRight));
    osdLayout->addWidget(cmbDatePos, 0, 3);

    chkTitleOsd = new QCheckBox(tr("Display Title Banner"), grpOsd);
    osdLayout->addWidget(chkTitleOsd, 1, 0);

    editTitleText = new QLineEdit("CAM-1 FUJINON SX800", grpOsd);
    osdLayout->addWidget(editTitleText, 1, 1, 1, 2);

    cmbTitlePos = new QComboBox(grpOsd);
    cmbTitlePos->addItem(tr("Top Left"), static_cast<int>(FujinonSX800::OsdPosition::TopLeft));
    cmbTitlePos->addItem(tr("Top Right"), static_cast<int>(FujinonSX800::OsdPosition::TopRight));
    cmbTitlePos->addItem(tr("Bottom Left"), static_cast<int>(FujinonSX800::OsdPosition::BottomLeft));
    cmbTitlePos->addItem(tr("Bottom Right"), static_cast<int>(FujinonSX800::OsdPosition::BottomRight));
    osdLayout->addWidget(cmbTitlePos, 1, 3);

    chkIdOsd = new QCheckBox(tr("Display Camera ID"), grpOsd);
    osdLayout->addWidget(chkIdOsd, 2, 0);

    cmbIdPos = new QComboBox(grpOsd);
    cmbIdPos->addItem(tr("Top Left"), static_cast<int>(FujinonSX800::OsdPosition::TopLeft));
    cmbIdPos->addItem(tr("Top Right"), static_cast<int>(FujinonSX800::OsdPosition::TopRight));
    cmbIdPos->addItem(tr("Bottom Left"), static_cast<int>(FujinonSX800::OsdPosition::BottomLeft));
    cmbIdPos->addItem(tr("Bottom Right"), static_cast<int>(FujinonSX800::OsdPosition::BottomRight));
    osdLayout->addWidget(cmbIdPos, 2, 3);

    chkCenterCrosshair = new QCheckBox(tr("Display Center Reticle / Crosshair"), grpOsd);
    osdLayout->addWidget(chkCenterCrosshair, 3, 0);

    chkAntialiasing = new QCheckBox(tr("OSD Font Antialiasing"), grpOsd);
    chkAntialiasing->setChecked(true);
    osdLayout->addWidget(chkAntialiasing, 4, 0);

    // RTC Clock Sync
    editRtcTime = new QDateTimeEdit(QDateTime::currentDateTime(), grpOsd);
    editRtcTime->setDisplayFormat("yyyy/MM/dd HH:mm:ss");
    btnSyncRtc = new QPushButton(tr("Sync Time to Camera"), grpOsd);
    btnSyncRtc->setObjectName("btnPrimary");
    osdLayout->addWidget(editRtcTime, 5, 0, 1, 2);
    osdLayout->addWidget(btnSyncRtc, 5, 2, 1, 2);

    mainLayout->addWidget(grpOsd);

    // ==================== 2. VIDEO OUTPUT FORMAT ====================
    auto* grpVideo = new QGroupBox(tr("HDMI / SDI Video Output Settings"), this);
    auto* videoLayout = new QGridLayout(grpVideo);
    videoLayout->setSpacing(10);

    videoLayout->addWidget(new QLabel(tr("Video Standard:"), grpVideo), 0, 0);
    cmbVideoStandard = new QComboBox(grpVideo);
    cmbVideoStandard->addItem("NTSC (60 Hz / 30 fps)", static_cast<int>(FujinonSX800::VideoStandard::NTSC));
    cmbVideoStandard->addItem("PAL (50 Hz / 25 fps)", static_cast<int>(FujinonSX800::VideoStandard::PAL));
    videoLayout->addWidget(cmbVideoStandard, 0, 1);

    videoLayout->addWidget(new QLabel(tr("HD Resolution & Framerate:"), grpVideo), 1, 0);
    cmbHdFormat = new QComboBox(grpVideo);
    cmbHdFormat->addItem("1080p @ 60 fps", static_cast<int>(FujinonSX800::HdFormat::HD1080p_60));
    cmbHdFormat->addItem("1080p @ 50 fps", static_cast<int>(FujinonSX800::HdFormat::HD1080p_50));
    cmbHdFormat->addItem("1080p @ 30 fps", static_cast<int>(FujinonSX800::HdFormat::HD1080p_30));
    cmbHdFormat->addItem("1080p @ 25 fps", static_cast<int>(FujinonSX800::HdFormat::HD1080p_25));
    cmbHdFormat->addItem("720p @ 60 fps", static_cast<int>(FujinonSX800::HdFormat::HD720p_60));
    cmbHdFormat->addItem("720p @ 50 fps", static_cast<int>(FujinonSX800::HdFormat::HD720p_50));
    videoLayout->addWidget(cmbHdFormat, 1, 1);

    videoLayout->addWidget(new QLabel(tr("Video Display Mode:"), grpVideo), 2, 0);
    cmbVideoDisplayMode = new QComboBox(grpVideo);
    cmbVideoDisplayMode->addItem(tr("Fit (Aspect Ratio Preserved)"), static_cast<int>(FujinonSX800::VideoDisplayMode::Fit));
    cmbVideoDisplayMode->addItem(tr("Fill (Full Screen)"), static_cast<int>(FujinonSX800::VideoDisplayMode::Fill));
    videoLayout->addWidget(cmbVideoDisplayMode, 2, 1);

    mainLayout->addWidget(grpVideo);
}

void OsdVideoTab::setupConnections()
{
    connect(chkDateTimeOsd, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setDateTimeOsd);
    connect(chkTitleOsd, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setTitleOsd);
    connect(chkIdOsd, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setIdOsd);
    connect(chkCenterCrosshair, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setCenterCrosshair);
    connect(chkAntialiasing, &QCheckBox::toggled, cam, &FujinonSX800Qt::QFujinonCamera::setAntialiasing);

    connect(cmbDatePos, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto pos = static_cast<FujinonSX800::OsdPosition>(cmbDatePos->currentData().toInt());
        cam->setOsdDatePosition(pos);
    });

    connect(cmbTitlePos, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto pos = static_cast<FujinonSX800::OsdPosition>(cmbTitlePos->currentData().toInt());
        cam->setOsdTitlePosition(pos);
    });

    connect(cmbIdPos, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto pos = static_cast<FujinonSX800::OsdPosition>(cmbIdPos->currentData().toInt());
        cam->setOsdIdPosition(pos);
    });

    connect(btnSyncRtc, &QPushButton::clicked, this, [this]() {
        cam->setRtcTime(editRtcTime->dateTime());
    });

    connect(cmbVideoStandard, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto std = static_cast<FujinonSX800::VideoStandard>(cmbVideoStandard->currentData().toInt());
        cam->setVideoStandard(std);
    });

    connect(cmbHdFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto fmt = static_cast<FujinonSX800::HdFormat>(cmbHdFormat->currentData().toInt());
        cam->setHdFormat(fmt);
    });

    connect(cmbVideoDisplayMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::VideoDisplayMode>(cmbVideoDisplayMode->currentData().toInt());
        cam->setVideoDisplayMode(mode);
    });
}

void OsdVideoTab::updateTelemetry(const FujinonSX800::CameraStatus& status)
{
    isUpdatingFromTelemetry = true;

    chkDateTimeOsd->setChecked(status.dateTimeDisplay);
    chkTitleOsd->setChecked(status.titleDisplay);
    chkIdOsd->setChecked(status.idDisplay);
    chkCenterCrosshair->setChecked(status.centerPositionDisplay);
    chkAntialiasing->setChecked(status.antialiasingEnabled);

    isUpdatingFromTelemetry = false;
}

} // namespace FujinonSX800App

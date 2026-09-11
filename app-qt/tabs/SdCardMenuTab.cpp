/// @file SdCardMenuTab.cpp
/// @brief Implementation of SD card and virtual OSD remote tab.

#include "SdCardMenuTab.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStyle>
#include <QVBoxLayout>

namespace FujinonSX800App {

SdCardMenuTab::SdCardMenuTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent)
    : QWidget(parent)
    , cam { camera }
{
    setupUi();
    setupConnections();
}

void SdCardMenuTab::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ==================== 1. SD CARD OPERATIONS ====================
    auto* grpSd = new QGroupBox(tr("SD Card Recording & Playback"), this);
    auto* sdLayout = new QVBoxLayout(grpSd);
    sdLayout->setSpacing(12);

    auto* recLayout = new QHBoxLayout();
    btnRecordLiveView = new QPushButton(tr("Record LiveView"), grpSd);
    btnRecordLiveView->setMinimumHeight(34);
    btnRecordLiveView->setObjectName("btnPrimary");

    lblRecordIndicator = new QLabel(tr("IDLE"), grpSd);
    lblRecordIndicator->setObjectName("lblTelemetry");

    recLayout->addWidget(btnRecordLiveView);
    recLayout->addWidget(lblRecordIndicator);
    recLayout->addStretch();
    sdLayout->addLayout(recLayout);

    auto* movieFileLayout = new QHBoxLayout();
    movieFileLayout->addWidget(new QLabel(tr("Movie File No:"), grpSd));
    spinMovieFileNo = new QSpinBox(grpSd);
    spinMovieFileNo->setRange(1, 9999);
    spinMovieFileNo->setValue(1);
    movieFileLayout->addWidget(spinMovieFileNo);

    movieFileLayout->addWidget(new QLabel(tr("Mode:"), grpSd));
    cmbPlaybackMode = new QComboBox(grpSd);
    cmbPlaybackMode->addItem(tr("Live View"), static_cast<int>(FujinonSX800::SdPlaybackMode::LiveView));
    cmbPlaybackMode->addItem(tr("Playback"), static_cast<int>(FujinonSX800::SdPlaybackMode::Playback));
    movieFileLayout->addWidget(cmbPlaybackMode);
    sdLayout->addLayout(movieFileLayout);

    // Media Playback Controls
    auto* playbackLayout = new QHBoxLayout();
    btnRewind = new QPushButton(tr("<< RW"), grpSd);
    btnPlay = new QPushButton(tr("Play >"), grpSd);
    btnPause = new QPushButton(tr("Pause ||"), grpSd);
    btnStopMovie = new QPushButton(tr("Stop []"), grpSd);
    btnFastForward = new QPushButton(tr("FF >>"), grpSd);

    playbackLayout->addWidget(btnRewind);
    playbackLayout->addWidget(btnPlay);
    playbackLayout->addWidget(btnPause);
    playbackLayout->addWidget(btnStopMovie);
    playbackLayout->addWidget(btnFastForward);
    sdLayout->addLayout(playbackLayout);

    btnFormatSd = new QPushButton(tr("Format SD Card..."), grpSd);
    btnFormatSd->setObjectName("btnDanger");
    sdLayout->addWidget(btnFormatSd);
    sdLayout->addStretch();

    mainLayout->addWidget(grpSd);

    // ==================== 2. VIRTUAL OSD REMOTE ====================
    auto* grpRemote = new QGroupBox(tr("Virtual OSD Menu Remote D-Pad"), this);
    auto* remoteLayout = new QGridLayout(grpRemote);
    remoteLayout->setSpacing(8);

    btnMenuUp = new QPushButton(tr("▲ Up"), grpRemote);
    btnMenuDown = new QPushButton(tr("▼ Down"), grpRemote);
    btnMenuLeft = new QPushButton(tr("◄ Left"), grpRemote);
    btnMenuRight = new QPushButton(tr("Right ►"), grpRemote);
    btnMenuOk = new QPushButton(tr("OK / Menu"), grpRemote);
    btnMenuOk->setObjectName("btnPrimary");
    btnMenuBack = new QPushButton(tr("Back"), grpRemote);

    btnMenuUp->setFixedSize(80, 36);
    btnMenuDown->setFixedSize(80, 36);
    btnMenuLeft->setFixedSize(80, 36);
    btnMenuRight->setFixedSize(80, 36);
    btnMenuOk->setFixedSize(80, 36);
    btnMenuBack->setFixedSize(80, 36);

    remoteLayout->addWidget(btnMenuUp, 0, 1, Qt::AlignCenter);
    remoteLayout->addWidget(btnMenuLeft, 1, 0, Qt::AlignCenter);
    remoteLayout->addWidget(btnMenuOk, 1, 1, Qt::AlignCenter);
    remoteLayout->addWidget(btnMenuRight, 1, 2, Qt::AlignCenter);
    remoteLayout->addWidget(btnMenuDown, 2, 1, Qt::AlignCenter);
    remoteLayout->addWidget(btnMenuBack, 2, 2, Qt::AlignCenter);

    mainLayout->addWidget(grpRemote);
}

void SdCardMenuTab::setupConnections()
{
    // Recording
    connect(btnRecordLiveView, &QPushButton::clicked, this, [this]() {
        isRecording = !isRecording;
        if (isRecording) {
            btnRecordLiveView->setText(tr("Stop Recording"));
            btnRecordLiveView->setObjectName("btnDanger");
            lblRecordIndicator->setText(tr("REC"));
            lblRecordIndicator->setStyleSheet("color: #ff7b72; font-weight: bold;");
        } else {
            btnRecordLiveView->setText(tr("Record LiveView"));
            btnRecordLiveView->setObjectName("btnPrimary");
            lblRecordIndicator->setText(tr("IDLE"));
            lblRecordIndicator->setStyleSheet("color: #8b949e;");
        }
        btnRecordLiveView->style()->unpolish(btnRecordLiveView);
        btnRecordLiveView->style()->polish(btnRecordLiveView);
    });

    // Media Playback
    connect(spinMovieFileNo, QOverload<int>::of(&QSpinBox::valueChanged), cam,
        &FujinonSX800Qt::QFujinonCamera::setSdMovieFile);
    connect(cmbPlaybackMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const auto mode = static_cast<FujinonSX800::SdPlaybackMode>(cmbPlaybackMode->currentData().toInt());
        cam->setSdPlaybackSpeed(mode);
    });
    connect(btnPlay, &QPushButton::clicked, this,
        [this]() { cam->setSdPlayback(FujinonSX800::SdPlaybackControl::PlayPause); });
    connect(btnPause, &QPushButton::clicked, this,
        [this]() { cam->setSdPlayback(FujinonSX800::SdPlaybackControl::PlayPause); });
    connect(btnStopMovie, &QPushButton::clicked, this,
        [this]() { cam->setSdPlayback(FujinonSX800::SdPlaybackControl::Stop); });
    connect(btnFastForward, &QPushButton::clicked, this,
        [this]() { cam->setSdPlayback(FujinonSX800::SdPlaybackControl::FastForward); });
    connect(btnRewind, &QPushButton::clicked, this,
        [this]() { cam->setSdPlayback(FujinonSX800::SdPlaybackControl::FastRewind); });

    // Format
    connect(btnFormatSd, &QPushButton::clicked, this, [this]() {
        const auto res = QMessageBox::warning(this, tr("Format SD Card"),
            tr("Are you sure you want to format the SD card? All recorded media will be lost."),
            QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            cam->formatSdCard();
        }
    });

    // Remote D-Pad
    connect(
        btnMenuUp, &QPushButton::clicked, this, [this]() { cam->sendMenuDirection(FujinonSX800::MenuDirection::Up); });
    connect(btnMenuDown, &QPushButton::clicked, this,
        [this]() { cam->sendMenuDirection(FujinonSX800::MenuDirection::Down); });
    connect(btnMenuLeft, &QPushButton::clicked, this,
        [this]() { cam->sendMenuDirection(FujinonSX800::MenuDirection::Left); });
    connect(btnMenuRight, &QPushButton::clicked, this,
        [this]() { cam->sendMenuDirection(FujinonSX800::MenuDirection::Right); });
    connect(btnMenuOk, &QPushButton::clicked, cam, &FujinonSX800Qt::QFujinonCamera::sendMenuEnter);
    connect(btnMenuBack, &QPushButton::clicked, cam, &FujinonSX800Qt::QFujinonCamera::sendMenuBack);
}

void SdCardMenuTab::updateTelemetry(const FujinonSX800::CameraStatus& status)
{
    Q_UNUSED(status);
}

} // namespace FujinonSX800App

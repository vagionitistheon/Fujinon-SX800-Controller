#pragma once

/// @file SdCardMenuTab.h
/// @brief UI Tab for SD card recording, movie playback, and virtual OSD D-pad navigation.

#include "CameraStatus.h"
#include "QFujinonCamera.h"

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

namespace FujinonSX800App {

/// @class SdCardMenuTab
/// @brief Controls SD card media operations and provides a virtual serial remote control.
class SdCardMenuTab : public QWidget {
    Q_OBJECT

public:
    explicit SdCardMenuTab(FujinonSX800Qt::QFujinonCamera* camera, QWidget* parent = nullptr);
    ~SdCardMenuTab() override = default;

public slots:
    void updateTelemetry(const FujinonSX800::CameraStatus& status);

private:
    void setupUi();
    void setupConnections();

    FujinonSX800Qt::QFujinonCamera* cam { nullptr };

    // SD Card Controls
    QPushButton* btnRecordLiveView { nullptr };
    QLabel* lblRecordIndicator { nullptr };
    QPushButton* btnFormatSd { nullptr };

    QPushButton* btnPlay { nullptr };
    QPushButton* btnPause { nullptr };
    QPushButton* btnStopMovie { nullptr };
    QPushButton* btnRewind { nullptr };
    QPushButton* btnFastForward { nullptr };

    // Virtual OSD Remote Keys
    QPushButton* btnMenuUp { nullptr };
    QPushButton* btnMenuDown { nullptr };
    QPushButton* btnMenuLeft { nullptr };
    QPushButton* btnMenuRight { nullptr };
    QPushButton* btnMenuOk { nullptr };
    QPushButton* btnMenuBack { nullptr };

    bool isRecording { false };
};

} // namespace FujinonSX800App

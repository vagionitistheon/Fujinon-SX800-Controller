#pragma once

/// @file QFujinonCamera.h
/// @brief Qt 6 QObject adapter wrapping FujinonCamera for GUI integration.

#include "CameraStatus.h"
#include "FujinonCamera.h"
#include "ITransport.h"
#include "PelcoDTypes.h"

#include <QByteArray>
#include <QObject>
#include <QString>

#include <memory>
#include <vector>

namespace FujinonSX800Qt {

class QFujinonCamera : public QObject {
    Q_OBJECT

public:
    explicit QFujinonCamera(
        std::shared_ptr<FujinonSX800::ITransport> transport = nullptr,
        std::uint8_t address = 1U,
        QObject* parent = nullptr);
    ~QFujinonCamera() override;

    void setTransport(std::shared_ptr<FujinonSX800::ITransport> transport, std::uint8_t address = 1U);
    [[nodiscard]] bool connectCamera();
    void disconnectCamera();
    [[nodiscard]] bool isConnected() const noexcept;
    [[nodiscard]] FujinonSX800::CameraStatus currentStatus() const;
    [[nodiscard]] FujinonSX800::FujinonCamera* coreCamera() const noexcept;

    // Lifecycle compatibility wrappers
    bool start() { return connectCamera(); }
    void stop() { disconnectCamera(); }

    void setAutoQueryOnConnect(bool enable);
    void setTelemetryPolling(bool enable, int intervalMs = 1000);
    void setQueryTimeoutMs(int timeoutMs);

signals:
    void statusUpdated(const FujinonSX800::CameraStatus& status);
    void trafficLogged(bool isTx, const QByteArray& packet, const QString& description);
    void frameLogged(const QByteArray& frame, bool isTx);
    void connectionStateChanged(bool connected);
    void queryTimeoutOccurred(const QString& queryTag);

public slots:
    // Motion & Zoom/Focus
    void zoomTele();
    void zoomWide();
    void zoomStop();
    void focusNear();
    void focusFar();
    void focusStop();
    void irisOpen();
    void irisClose();
    void irisStop();
    void setZoomPosition(int pulse);
    void setFocusPosition(int pulse);
    void setIrisPosition(int pos);
    void setZoomSpeed(int speed);
    void setFocusSpeed(int speed);

    // Auto Focus & Exposure
    void setOnePushAf();
    void triggerOnePushAf() { setOnePushAf(); }
    void setAutoFocus(bool enable);
    void setAutoFocusMode(FujinonSX800::AutoFocusMode mode);
    void setAutoIris(bool enable);
    void setManualIris(int pos) { setIrisPosition(pos); }
    void setAgc(bool enable);
    void setBlc(bool enable);
    void setManualShutter(int speedIdx);
    void setManualIso(int isoIdx);
    void setAfArea(FujinonSX800::AfArea area);
    void setAfSensitivity(FujinonSX800::AfSensitivity sens);

    // Day / Night & Filters
    void setDayNightMode(FujinonSX800::DayNightMode mode);
    void setDayNightThresholds(int d2n, int n2d);
    void setDayNightDelay(int sec);
    void setOpticalFilterDay(FujinonSX800::OpticalFilter filter);
    void setOpticalFilterNight(FujinonSX800::OpticalFilter filter);
    void setIrWavelength(FujinonSX800::IrWavelength wl);

    // Image Quality & Processing
    void setVlcFilter(FujinonSX800::VlcFilterMode mode);
    void setDefog(FujinonSX800::DefogMode mode);
    void setDeHeatHaze(FujinonSX800::DeHeatHazeMode mode);
    void setOpticalStabilization(FujinonSX800::OpticalStabilization mode);
    void setStabilization(FujinonSX800::OpticalStabilization mode);
    void setWdr(FujinonSX800::WdrMode mode);
    void setBrightness(int val);
    void setBrightnessFine(int val);
    void setContrast(int val);
    void setContrastFine(int val);
    void setSaturation(int val);
    void setSaturationFine(int val);
    void setSharpness(int val);
    void setSharpnessFine(int val);
    void setWhiteBalance(FujinonSX800::WhiteBalanceMode mode);
    void setColorTemperature(int kelvin);
    void setWbColorTemp(int kelvin);
    void setWbShiftRedFine(int shift);
    void setWbShiftBlueFine(int shift);
    void setDigitalZoom(FujinonSX800::DigitalZoomMode mode);
    void setDigitalZoom(FujinonSX800::DigitalZoomMode mode, int mag);
    void setDigitalZoomStep(FujinonSX800::DigitalZoomStep step);
    void setNoiseReduction(FujinonSX800::NoiseReductionLevel level);

    // OSD & Video
    void setDateTimeOsd(bool enable);
    void setTitleOsd(bool enable);
    void setIdOsd(bool enable);
    void setCenterCrosshair(bool enable);
    void setAntialiasing(bool enable);
    void setVideoStandard(FujinonSX800::VideoStandard standard);
    void setHdFormat(FujinonSX800::HdFormat format);

    // Menu & System
    void sendMenuKey(FujinonSX800::MenuKey key);
    void sendMenuDirection(FujinonSX800::MenuDirection dir);
    void sendMenuEnter();
    void sendMenuBack();
    void formatSdCard();
    void setRs485Termination(bool enable);
    void setLanguage(FujinonSX800::Language lang);
    void refreshStatus();
    void resetDefaults(int preset = 0);
    void rebootCamera();
    void sendRawHex(const QByteArray& hexData);
    void sendRawHexPacket(const QString& hex);
    void refreshAll();
    void queryTemperature();

private:
    [[nodiscard]] static QString describePacket(bool isTx, const std::vector<std::uint8_t>& frame);

    std::shared_ptr<FujinonSX800::ITransport> m_transport;
    std::unique_ptr<FujinonSX800::FujinonCamera> m_camera;
    std::uint8_t m_address { 1U };
};

} // namespace FujinonSX800Qt

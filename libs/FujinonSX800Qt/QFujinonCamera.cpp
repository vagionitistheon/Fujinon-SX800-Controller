#include "QFujinonCamera.h"
#include "ProtocolParser.h"

#include <QMetaObject>

namespace FujinonSX800Qt {

QFujinonCamera::QFujinonCamera(
    std::shared_ptr<FujinonSX800::ITransport> transport,
    std::uint8_t address,
    QObject* parent)
    : QObject(parent)
    , m_transport { std::move(transport) }
    , m_address { address }
{
}

QFujinonCamera::~QFujinonCamera()
{
    disconnectCamera();
}

void QFujinonCamera::setTransport(
    std::shared_ptr<FujinonSX800::ITransport> transport, std::uint8_t address)
{
    disconnectCamera();
    m_transport = std::move(transport);
    m_address = address;
}

bool QFujinonCamera::connectCamera()
{
    if (!m_transport) {
        return false;
    }

    m_camera = std::make_unique<FujinonSX800::FujinonCamera>(m_transport, m_address);

    m_camera->addStatusCallback([this](const FujinonSX800::CameraStatus& status) {
        QMetaObject::invokeMethod(this, [this, status] {
            emit statusUpdated(status);
        });
    });

    m_camera->addTrafficCallback(
        [this](bool isTx, const std::vector<std::uint8_t>& frame) {
            const QByteArray bytes(
                reinterpret_cast<const char*>(frame.data()),
                static_cast<int>(frame.size()));
            const QString desc = describePacket(isTx, frame);

            QMetaObject::invokeMethod(this, [this, isTx, bytes, desc] {
                emit trafficLogged(isTx, bytes, desc);
                emit frameLogged(bytes, isTx);
            });
        });

    const bool ok = m_camera->start();
    emit connectionStateChanged(ok);
    return ok;
}

void QFujinonCamera::disconnectCamera()
{
    if (m_camera) {
        m_camera->stop();
        m_camera.reset();
        emit connectionStateChanged(false);
    }
}

bool QFujinonCamera::isConnected() const noexcept
{
    return m_camera && m_camera->isConnected();
}

FujinonSX800::CameraStatus QFujinonCamera::currentStatus() const
{
    if (m_camera) {
        return m_camera->getStatus();
    }
    return {};
}

FujinonSX800::FujinonCamera* QFujinonCamera::coreCamera() const noexcept
{
    return m_camera.get();
}

void QFujinonCamera::zoomTele()
{
    if (m_camera) {
        m_camera->zoomTele();
    }
}

void QFujinonCamera::zoomWide()
{
    if (m_camera) {
        m_camera->zoomWide();
    }
}

void QFujinonCamera::zoomStop()
{
    if (m_camera) {
        m_camera->zoomStop();
    }
}

void QFujinonCamera::focusNear()
{
    if (m_camera) {
        m_camera->focusNear();
    }
}

void QFujinonCamera::focusFar()
{
    if (m_camera) {
        m_camera->focusFar();
    }
}

void QFujinonCamera::focusStop()
{
    if (m_camera) {
        m_camera->focusStop();
    }
}

void QFujinonCamera::irisOpen()
{
    if (m_camera) {
        m_camera->irisOpen();
    }
}

void QFujinonCamera::irisClose()
{
    if (m_camera) {
        m_camera->irisClose();
    }
}

void QFujinonCamera::irisStop()
{
    if (m_camera) {
        m_camera->irisStop();
    }
}

void QFujinonCamera::setZoomPosition(int pulse)
{
    if (m_camera) {
        m_camera->setZoomPosition(static_cast<std::uint16_t>(pulse));
    }
}

void QFujinonCamera::setFocusPosition(int pulse)
{
    if (m_camera) {
        m_camera->setFocusPosition(static_cast<std::uint16_t>(pulse));
    }
}

void QFujinonCamera::setIrisPosition(int pos)
{
    if (m_camera) {
        m_camera->setIrisPosition(static_cast<std::uint16_t>(pos));
    }
}

void QFujinonCamera::setZoomSpeed(int speed)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetZoomSpeed(static_cast<std::uint8_t>(speed)));
    }
}

void QFujinonCamera::setFocusSpeed(int speed)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetFocusSpeed(static_cast<std::uint8_t>(speed)));
    }
}

void QFujinonCamera::setOnePushAf()
{
    if (m_camera) {
        m_camera->setOnePushAf();
    }
}

void QFujinonCamera::setAutoFocus(bool enable)
{
    setAutoFocusMode(enable ? FujinonSX800::AutoFocusMode::On : FujinonSX800::AutoFocusMode::Off);
}

void QFujinonCamera::setAutoFocusMode(FujinonSX800::AutoFocusMode mode)
{
    if (m_camera) {
        m_camera->setAutoFocus(mode);
    }
}

void QFujinonCamera::setAutoIris(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetAutoIris(enable));
    }
}

void QFujinonCamera::setAgc(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetAgc(enable));
    }
}

void QFujinonCamera::setBlc(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetBlc(enable));
    }
}

void QFujinonCamera::setManualShutter(int speedIdx)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetShutterSpeed(static_cast<FujinonSX800::ShutterSpeed>(speedIdx)));
    }
}

void QFujinonCamera::setManualIso(int isoIdx)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetIso(static_cast<FujinonSX800::IsoSensitivity>(isoIdx)));
    }
}

void QFujinonCamera::setAfArea(FujinonSX800::AfArea area)
{
    if (m_camera) {
        m_camera->setAfArea(area);
    }
}

void QFujinonCamera::setAfSensitivity(FujinonSX800::AfSensitivity sens)
{
    if (m_camera) {
        m_camera->setAfSensitivity(sens);
    }
}

void QFujinonCamera::setDayNightMode(FujinonSX800::DayNightMode mode)
{
    if (m_camera) {
        m_camera->setDayNightMode(mode);
    }
}

void QFujinonCamera::setDayNightThresholds(int d2n, int n2d)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetDayNightThreshold(
            static_cast<std::uint8_t>(d2n), static_cast<std::uint8_t>(n2d)));
    }
}

void QFujinonCamera::setDayNightDelay(int sec)
{
    if (m_camera) {
        // Original Command 9 delay parameter
        m_camera->sendRawFrame(FujinonSX800::PelcoDFrame::createFrame(
            m_address, 0xF1U, 0x05U, 0x00U, static_cast<std::uint8_t>(sec)));
    }
}

void QFujinonCamera::setOpticalFilterDay(FujinonSX800::OpticalFilter filter)
{
    if (m_camera) {
        m_camera->setOpticalFilterDay(filter);
    }
}

void QFujinonCamera::setOpticalFilterNight(FujinonSX800::OpticalFilter filter)
{
    if (m_camera) {
        m_camera->setOpticalFilterNight(filter);
    }
}

void QFujinonCamera::setIrWavelength(FujinonSX800::IrWavelength wl)
{
    if (m_camera) {
        m_camera->setIrWavelength(wl);
    }
}

void QFujinonCamera::setVlcFilter(FujinonSX800::VlcFilterMode mode)
{
    if (m_camera) {
        m_camera->setVlcFilter(mode);
    }
}

void QFujinonCamera::setDefog(FujinonSX800::DefogMode mode)
{
    if (m_camera) {
        m_camera->setDefog(mode);
    }
}

void QFujinonCamera::setDeHeatHaze(FujinonSX800::DeHeatHazeMode mode)
{
    if (m_camera) {
        m_camera->setDeHeatHaze(mode);
    }
}

void QFujinonCamera::setOpticalStabilization(FujinonSX800::OpticalStabilization mode)
{
    if (m_camera) {
        m_camera->setOpticalStabilization(mode);
    }
}

void QFujinonCamera::setStabilization(FujinonSX800::OpticalStabilization mode)
{
    setOpticalStabilization(mode);
}

void QFujinonCamera::setWdr(FujinonSX800::WdrMode mode)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetWdr(mode));
    }
}

void QFujinonCamera::setBrightness(int val)
{
    if (m_camera) {
        m_camera->setBrightness(static_cast<std::uint8_t>(val));
    }
}

void QFujinonCamera::setBrightnessFine(int val)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetFineBrightness(static_cast<std::int8_t>(val)));
    }
}

void QFujinonCamera::setContrast(int val)
{
    if (m_camera) {
        m_camera->setContrast(static_cast<std::uint8_t>(val));
    }
}

void QFujinonCamera::setContrastFine(int val)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetFineContrast(static_cast<std::int8_t>(val)));
    }
}

void QFujinonCamera::setSaturation(int val)
{
    if (m_camera) {
        m_camera->setSaturation(static_cast<std::uint8_t>(val));
    }
}

void QFujinonCamera::setSaturationFine(int val)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetFineSaturation(static_cast<std::int8_t>(val)));
    }
}

void QFujinonCamera::setSharpness(int val)
{
    if (m_camera) {
        m_camera->setSharpness(static_cast<std::uint8_t>(val));
    }
}

void QFujinonCamera::setSharpnessFine(int val)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetFineSharpness(static_cast<std::int8_t>(val)));
    }
}

void QFujinonCamera::setWhiteBalance(FujinonSX800::WhiteBalanceMode mode)
{
    if (m_camera) {
        m_camera->setWhiteBalance(mode);
    }
}

void QFujinonCamera::setColorTemperature(int kelvin)
{
    if (m_camera) {
        m_camera->setColorTemperature(static_cast<std::uint16_t>(kelvin));
    }
}

void QFujinonCamera::setWbColorTemp(int kelvin)
{
    setColorTemperature(kelvin);
}

void QFujinonCamera::setWbShiftRedFine(int shift)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetWbRedShift(static_cast<std::int8_t>(shift)));
    }
}

void QFujinonCamera::setWbShiftBlueFine(int shift)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetWbBlueShift(static_cast<std::int8_t>(shift)));
    }
}

void QFujinonCamera::setDigitalZoom(FujinonSX800::DigitalZoomMode mode)
{
    setDigitalZoom(mode, 0);
}

void QFujinonCamera::setDigitalZoom(FujinonSX800::DigitalZoomMode mode, int mag)
{
    if (m_camera) {
        m_camera->setDigitalZoom(mode, static_cast<std::uint8_t>(mag));
    }
}

void QFujinonCamera::setDigitalZoomStep(FujinonSX800::DigitalZoomStep step)
{
    if (m_camera) {
        // Step command in Pelco-D Original Command 2
        m_camera->sendRawFrame(FujinonSX800::PelcoDFrame::createFrame(
            m_address, 0xF0U, 0x37U, 0x00U, static_cast<std::uint8_t>(step)));
    }
}

void QFujinonCamera::setNoiseReduction(FujinonSX800::NoiseReductionLevel level)
{
    if (m_camera) {
        m_camera->setNoiseReduction(level);
    }
}

void QFujinonCamera::setDateTimeOsd(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetDateTimeDisplay(enable));
    }
}

void QFujinonCamera::setTitleOsd(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetTitleDisplay(enable));
    }
}

void QFujinonCamera::setIdOsd(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetIdDisplay(enable));
    }
}

void QFujinonCamera::setCenterCrosshair(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetReticleDisplay(enable));
    }
}

void QFujinonCamera::setAntialiasing(bool enable)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetAntialiasing(enable));
    }
}

void QFujinonCamera::setVideoStandard(FujinonSX800::VideoStandard standard)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetVideoStandard(standard));
    }
}

void QFujinonCamera::setHdFormat(FujinonSX800::HdFormat format)
{
    if (m_camera) {
        FujinonSX800::ProtocolBuilder pb(m_address);
        m_camera->sendRawFrame(pb.buildSetHdFormat(format));
    }
}

void QFujinonCamera::sendMenuKey(FujinonSX800::MenuKey key)
{
    if (m_camera) {
        m_camera->sendMenuKey(key);
    }
}

void QFujinonCamera::sendMenuDirection(FujinonSX800::MenuDirection dir)
{
    if (m_camera) {
        m_camera->sendMenuKey(dir);
    }
}

void QFujinonCamera::sendMenuEnter()
{
    if (m_camera) {
        m_camera->sendMenuKey(FujinonSX800::MenuKey::Ok);
    }
}

void QFujinonCamera::sendMenuBack()
{
    if (m_camera) {
        m_camera->sendMenuKey(FujinonSX800::MenuKey::Back);
    }
}

void QFujinonCamera::formatSdCard()
{
    if (m_camera) {
        m_camera->sendRawFrame(FujinonSX800::PelcoDFrame::createFrame(m_address, 0xF0U, 0x6DU, 0x00U, 0x01U));
    }
}

void QFujinonCamera::setRs485Termination(bool enable)
{
    if (m_camera) {
        m_camera->setTermination(enable);
    }
}

void QFujinonCamera::setLanguage(FujinonSX800::Language lang)
{
    if (m_camera) {
        m_camera->sendRawFrame(FujinonSX800::PelcoDFrame::createFrame(
            m_address, 0xF0U, 0x75U, 0x00U, static_cast<std::uint8_t>(lang)));
    }
}

void QFujinonCamera::refreshStatus()
{
    refreshAll();
}

void QFujinonCamera::resetDefaults(int preset)
{
    (void)preset;
    if (m_camera) {
        m_camera->factoryReset();
    }
}

void QFujinonCamera::rebootCamera()
{
    if (m_camera) {
        m_camera->reboot();
    }
}

void QFujinonCamera::sendRawHex(const QByteArray& hexData)
{
    if (!m_camera) {
        return;
    }
    std::vector<std::uint8_t> frame(hexData.begin(), hexData.end());
    m_camera->sendRawFrame(frame);
}

void QFujinonCamera::sendRawHexPacket(const QString& hex)
{
    if (!m_camera) {
        return;
    }
    const QByteArray clean = QByteArray::fromHex(hex.toUtf8());
    sendRawHex(clean);
}

void QFujinonCamera::refreshAll()
{
    if (m_camera) {
        m_camera->queryAll();
    }
}

QString QFujinonCamera::describePacket(bool isTx, const std::vector<std::uint8_t>& frame)
{
    return QString::fromStdString(FujinonSX800::ProtocolParser::describeFrame(frame, isTx));
}

} // namespace FujinonSX800Qt

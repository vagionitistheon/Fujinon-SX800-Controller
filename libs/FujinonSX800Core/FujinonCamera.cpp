#include "FujinonCamera.h"
#include "ProtocolParser.h"

#include <chrono>
#include <glog/logging.h>

namespace FujinonSX800 {

FujinonCamera::FujinonCamera(std::shared_ptr<ITransport> transport, std::uint8_t address)
    : m_transport { std::move(transport) }
    , m_builder { address }
    , m_address { address }
{
    m_status.rs485Address = address;
}

FujinonCamera::~FujinonCamera()
{
    stop();
}

bool FujinonCamera::start()
{
    if (!m_transport) {
        return false;
    }

    m_transport->setDataCallback([this](const std::uint8_t* data, std::size_t size) { onDataReceived(data, size); });
    m_transport->setStateCallback(
        [this](TransportState state, const std::string& errorMsg) { onTransportStateChanged(state, errorMsg); });

    if (!m_transport->isOpen()) {
        if (!m_transport->open()) {
            m_transport->setDataCallback(nullptr);
            m_transport->setStateCallback(nullptr);
            return false;
        }
    }

    m_connected = true;
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_status.isConnected = true;
        m_status.transportState = TransportState::Connected;
        m_status.transportError.clear();
    }

    LOG(INFO) << "Starting FujinonCamera controller (address: " << static_cast<int>(m_address.load()) << ")";

    m_rxRing.clear();
    m_running = true;
    m_rxThread = std::thread(&FujinonCamera::rxLoop, this);
    m_workerThread = std::thread(&FujinonCamera::workerLoop, this);

    if (m_autoQueryOnConnect.load()) {
        queryAll();
    }

    return true;
}

void FujinonCamera::stop()
{
    LOG(INFO) << "Stopping FujinonCamera controller";
    m_running = false;
    m_queueCv.notify_all();
    m_rxCv.notify_all();
    m_responseCv.notify_all();
    m_reconnectCv.notify_all();

    if (m_rxThread.joinable()) {
        m_rxThread.join();
    }
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    if (m_reconnectThread.joinable()) {
        m_reconnectThread.join();
    }
    m_awaitingResponse = false;

    if (m_transport) {
        m_transport->setDataCallback(nullptr);
        m_transport->setStateCallback(nullptr);
    }
    if (m_transport && m_transport->isOpen()) {
        m_transport->close();
    }

    m_connected = false;
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_status.isConnected = false;
        m_status.transportState = TransportState::Disconnected;
        m_status.transportError.clear();
    }
}

bool FujinonCamera::isConnected() const noexcept
{
    return m_running && m_connected.load();
}

void FujinonCamera::setAddress(std::uint8_t address)
{
    m_address.store(address);
    m_builder.setAddress(address);
    std::lock_guard<std::mutex> lock(m_statusMutex);
    m_status.rs485Address = address;
}

std::uint8_t FujinonCamera::getAddress() const noexcept
{
    return m_address.load();
}

void FujinonCamera::addStatusCallback(StatusCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_statusCallbacks.push_back(std::move(cb));
}

void FujinonCamera::addTrafficCallback(TrafficCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_trafficCallbacks.push_back(std::move(cb));
}

void FujinonCamera::addTimeoutCallback(TimeoutCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_timeoutCallbacks.push_back(std::move(cb));
}

void FujinonCamera::addTransportStateCallback(TransportStateCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_transportStateCallbacks.push_back(std::move(cb));
}

void FujinonCamera::setAutoQueryOnConnect(bool enable) noexcept
{
    m_autoQueryOnConnect.store(enable);
}

bool FujinonCamera::getAutoQueryOnConnect() const noexcept
{
    return m_autoQueryOnConnect.load();
}

void FujinonCamera::setTelemetryPolling(bool enable, std::uint32_t intervalMs) noexcept
{
    m_telemetryPolling.store(enable);
    m_pollIntervalMs.store((intervalMs > 0U) ? intervalMs : 1000U);
    m_queueCv.notify_one();
}

bool FujinonCamera::getTelemetryPolling() const noexcept
{
    return m_telemetryPolling.load();
}

void FujinonCamera::setQueryTimeoutMs(std::uint32_t timeoutMs) noexcept
{
    m_queryTimeoutMs.store((timeoutMs > 0U) ? timeoutMs : 1000U);
}

std::uint32_t FujinonCamera::getQueryTimeoutMs() const noexcept
{
    return m_queryTimeoutMs.load();
}

void FujinonCamera::setAutoReconnect(bool enable) noexcept
{
    m_autoReconnect = enable;
    if (!enable) {
        m_reconnectCv.notify_all();
    }
}

bool FujinonCamera::getAutoReconnect() const noexcept
{
    return m_autoReconnect.load();
}

void FujinonCamera::checkQueryTimeout()
{
    if (!m_awaitingResponse.load()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_querySentTime).count();
    const auto queryTimeoutMs = m_queryTimeoutMs.load();
    if (elapsed >= static_cast<long long>(queryTimeoutMs)) {
        m_awaitingResponse = false;
        m_responseCv.notify_all();
        m_rxCv.notify_one();
        std::string tag;
        {
            std::lock_guard<std::mutex> lock(m_statusMutex);
            tag = m_pendingQueryTag;
        }

        LOG(WARNING) << "Query timeout: No response received for query '" << tag << "' within " << queryTimeoutMs
                     << " ms";

        std::vector<TimeoutCallback> cbs;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            cbs = m_timeoutCallbacks;
        }
        for (const auto& cb : cbs) {
            if (cb) {
                cb(tag);
            }
        }
    }
}

CameraStatus FujinonCamera::getStatus() const
{
    std::lock_guard<std::mutex> lock(m_statusMutex);
    return m_status;
}

void FujinonCamera::enqueueCommand(const std::vector<std::uint8_t>& frame, std::string queryTag)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_commandQueue.push_back({ frame, std::move(queryTag) });
    }
    m_queueCv.notify_one();
}

void FujinonCamera::enqueueTelemetryQueries()
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    bool hasZoom { false };
    bool hasFocus { false };
    bool hasLens { false };

    for (const auto& item : m_commandQueue) {
        if (item.queryTag == "QueryZoom") {
            hasZoom = true;
        } else if (item.queryTag == "QueryFocus") {
            hasFocus = true;
        } else if (item.queryTag == "QueryLens") {
            hasLens = true;
        }
    }

    if (!hasZoom) {
        m_commandQueue.push_back({ m_builder.buildQueryZoomPosition(), "QueryZoom" });
    }
    if (!hasFocus) {
        m_commandQueue.push_back({ m_builder.buildQueryFocusPosition(), "QueryFocus" });
    }
    if (!hasLens) {
        m_commandQueue.push_back({ m_builder.buildQueryLensStatus(), "QueryLens" });
    }
}

void FujinonCamera::workerLoop()
{
    bool lastPollingActive { false };
    auto nextPollTime = std::chrono::steady_clock::now();

    while (m_running) {
        checkQueryTimeout();

        const bool pollingActive = m_telemetryPolling.load(std::memory_order_relaxed) && isConnected();
        if (pollingActive) {
            const auto now = std::chrono::steady_clock::now();
            if (!lastPollingActive || now >= nextPollTime) {
                enqueueTelemetryQueries();
                nextPollTime = now + std::chrono::milliseconds(m_pollIntervalMs.load(std::memory_order_relaxed));
            }
        }
        lastPollingActive = pollingActive;

        CommandItem item;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            if (m_commandQueue.empty() && m_running) {
                if (m_telemetryPolling.load(std::memory_order_relaxed) && isConnected()) {
                    const auto now = std::chrono::steady_clock::now();
                    const auto waitDuration = (nextPollTime > now)
                        ? std::chrono::duration_cast<std::chrono::milliseconds>(nextPollTime - now)
                        : std::chrono::milliseconds(0);
                    m_queueCv.wait_for(lock, waitDuration, [this] { return !m_commandQueue.empty() || !m_running; });
                } else {
                    m_queueCv.wait_for(
                        lock, std::chrono::milliseconds(100), [this] { return !m_commandQueue.empty() || !m_running; });
                }
            }

            if (!m_running) {
                break;
            }
            if (m_commandQueue.empty()) {
                continue;
            }

            item = std::move(m_commandQueue.front());
            m_commandQueue.pop_front();
        }

        if (m_transport && m_transport->isOpen() && !item.frame.empty()) {
            {
                std::lock_guard<std::mutex> lock(m_statusMutex);
                m_lastQueryTag = item.queryTag;
                if (!item.queryTag.empty()) {
                    m_pendingQueryTag = item.queryTag;
                    m_querySentTime = std::chrono::steady_clock::now();
                    m_awaitingResponse = true;
                }
            }

            VLOG(1) << "Sending command (" << item.frame.size() << " bytes, tag: '" << item.queryTag << "')";
            const bool sendSuccess = m_transport->sendData(item.frame);
            if (!sendSuccess) {
                LOG(WARNING) << "Failed to transmit frame across transport.";
            }

            // Notify TX callbacks
            std::vector<TrafficCallback> tbs;
            {
                std::lock_guard<std::mutex> lock(m_callbackMutex);
                tbs = m_trafficCallbacks;
            }
            for (const auto& cb : tbs) {
                if (cb) {
                    cb(true, item.frame);
                }
            }

            if (!item.queryTag.empty()) {
                {
                    std::unique_lock<std::mutex> lock(m_statusMutex);
                    m_responseCv.wait_for(lock, std::chrono::milliseconds(m_queryTimeoutMs.load()),
                        [this] { return !m_awaitingResponse.load() || !m_running; });
                }
                checkQueryTimeout();
            }
        }

        // 20ms inter-command pacing delay per Pelco-D spec
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void FujinonCamera::startReconnect()
{
    std::lock_guard<std::mutex> lock(m_reconnectMutex);
    if (m_reconnectActive.load() || !m_running || !m_autoReconnect) {
        return;
    }

    if (m_reconnectThread.joinable()) {
        m_reconnectThread.join();
    }

    m_reconnectActive = true;
    m_reconnectThread = std::thread(&FujinonCamera::reconnectLoop, this);
}

void FujinonCamera::reconnectLoop()
{
    constexpr std::uint32_t initialDelayMs { 500U };
    constexpr std::uint32_t maxDelayMs { 30000U };
    std::uint32_t delayMs { initialDelayMs };

    while (m_running && m_autoReconnect) {
        std::unique_lock<std::mutex> lock(m_reconnectMutex);
        const bool cancelled = m_reconnectCv.wait_for(
            lock, std::chrono::milliseconds(delayMs), [this] { return !m_running || !m_autoReconnect; });
        lock.unlock();

        if (cancelled || !m_running || !m_autoReconnect) {
            break;
        }

        if (m_transport && m_transport->isOpen()) {
            m_transport->close();
        }

        if (m_transport && m_transport->open()) {
            break;
        }

        delayMs = std::min(delayMs * 2U, maxDelayMs);
    }

    m_reconnectActive = false;
}

void FujinonCamera::onDataReceived(const std::uint8_t* data, std::size_t size)
{
    if (data != nullptr && size > 0U) {
        if (!m_rxRing.writeExact(data, size)) {
            LOG(WARNING) << "RX ring buffer overflow: dropped " << size
                         << " bytes (available capacity: " << m_rxRing.availableWrite() << ")";
            m_rxOverflowDrops.fetch_add(size, std::memory_order_relaxed);
            return;
        }
        VLOG(2) << "Wrote " << size << " bytes to RX ring buffer";
        m_rxCv.notify_one();
    }
}

void FujinonCamera::onTransportStateChanged(TransportState state, const std::string& errorMsg)
{
    const bool connected = state == TransportState::Connected;
    m_connected = connected;

    CameraStatus status;
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_status.isConnected = connected;
        m_status.transportState = state;
        m_status.transportError = errorMsg;
        status = m_status;
    }

    std::vector<StatusCallback> statusCallbacks;
    std::vector<TransportStateCallback> stateCallbacks;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        statusCallbacks = m_statusCallbacks;
        stateCallbacks = m_transportStateCallbacks;
    }

    for (const auto& callback : statusCallbacks) {
        if (callback) {
            callback(status);
        }
    }
    for (const auto& callback : stateCallbacks) {
        if (callback) {
            callback(state, errorMsg);
        }
    }

    if ((state == TransportState::Disconnected || state == TransportState::Error) && m_running && m_autoReconnect) {
        startReconnect();
    }
}

bool FujinonCamera::is18ByteQuery(std::string_view tag) noexcept
{
    return tag == "QuerySerial";
}

void FujinonCamera::rxLoop()
{
    while (m_running) {
        {
            std::unique_lock<std::mutex> lock(m_rxMutex);
            m_rxCv.wait_for(lock, std::chrono::milliseconds(50), [this] {
                if (!m_running) {
                    return true;
                }
                const std::size_t avail = m_rxRing.availableRead();
                if (avail == 0U) {
                    return false;
                }
                if (m_awaitingResponse.load(std::memory_order_relaxed)) {
                    std::lock_guard<std::mutex> statusLock(m_statusMutex);
                    if (m_awaitingResponse.load(std::memory_order_relaxed) && is18ByteQuery(m_pendingQueryTag)) {
                        return avail >= PelcoDFrame::QueryResponseSize;
                    }
                }
                return avail >= PelcoDFrame::GeneralResponseSize;
            });
        }

        if (!m_running) {
            break;
        }

        while (m_running) {
            const std::size_t syncOffset = m_rxRing.findByte(PelcoDFrame::SyncByte);
            if (syncOffset == decltype(m_rxRing)::npos) {
                m_rxRing.advanceRead(m_rxRing.availableRead());
                break;
            }

            if (syncOffset > 0) {
                m_rxRing.advanceRead(syncOffset);
            }

            const std::size_t available = m_rxRing.availableRead();
            if (available < PelcoDFrame::GeneralResponseSize) {
                break;
            }

            bool awaiting18ByteQuery { false };
            bool awaitingAnyQuery { false };
            if (m_awaitingResponse.load(std::memory_order_relaxed)) {
                std::lock_guard<std::mutex> lock(m_statusMutex);
                if (m_awaitingResponse.load(std::memory_order_relaxed)) {
                    awaitingAnyQuery = true;
                    awaiting18ByteQuery = is18ByteQuery(m_pendingQueryTag);
                }
            }

            if (awaiting18ByteQuery && available < PelcoDFrame::QueryResponseSize) {
                break;
            }

            constexpr std::size_t candidateSizes[]
                = { PelcoDFrame::QueryResponseSize, PelcoDFrame::StandardFrameSize, PelcoDFrame::GeneralResponseSize };

            bool frameExtracted = false;
            std::array<std::uint8_t, PelcoDFrame::QueryResponseSize> peekBuf {};

            for (const std::size_t candidateSize : candidateSizes) {
                if (awaiting18ByteQuery && candidateSize < PelcoDFrame::QueryResponseSize) {
                    continue;
                }
                if (awaitingAnyQuery && candidateSize == PelcoDFrame::GeneralResponseSize) {
                    continue;
                }

                if (available >= candidateSize) {
                    if (m_rxRing.peekBytes(peekBuf.data(), candidateSize)) {
                        if (candidateSize == PelcoDFrame::GeneralResponseSize && peekBuf[2] != 0x00U) {
                            continue;
                        }
                        if (candidateSize == PelcoDFrame::StandardFrameSize
                            && !PelcoDFrame::isValidOpcode(peekBuf[2])) {
                            continue;
                        }

                        const std::uint8_t expectedCksm = peekBuf[candidateSize - 1U];
                        const std::uint8_t computedCksm
                            = PelcoDFrame::calculateChecksum(&peekBuf[1], candidateSize - 2U);

                        if (expectedCksm == computedCksm) {
                            std::vector<std::uint8_t> frame(peekBuf.begin(), peekBuf.begin() + candidateSize);
                            m_rxRing.advanceRead(candidateSize);
                            frameExtracted = true;
                            dispatchFrame(frame);
                            break;
                        }
                    }
                }
            }

            if (!frameExtracted) {
                if (available < PelcoDFrame::QueryResponseSize) {
                    break;
                }
                m_rxRing.advanceRead(1);
            }
        }
    }
}

void FujinonCamera::dispatchFrame(const std::vector<std::uint8_t>& frame)
{
    std::vector<TrafficCallback> tbs;
    std::vector<StatusCallback> sbs;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        tbs = m_trafficCallbacks;
        sbs = m_statusCallbacks;
    }

    for (const auto& cb : tbs) {
        if (cb) {
            cb(false, frame);
        }
    }

    CameraStatus updatedStatus;
    std::string qTag;
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        qTag = m_lastQueryTag;
        updatedStatus = m_status;
    }

    if (ProtocolParser::parsePacket(frame, updatedStatus, qTag)) {
        m_awaitingResponse = false;
        m_responseCv.notify_all();
        VLOG(1) << "Packet parsed successfully, updating status (tag: '" << qTag << "')";
        {
            std::lock_guard<std::mutex> lock(m_statusMutex);
            m_status = updatedStatus;
        }
        for (const auto& cb : sbs) {
            if (cb) {
                cb(updatedStatus);
            }
        }
    }
}

void FujinonCamera::zoomTele()
{
    enqueueCommand(m_builder.buildZoom(ZoomDirection::Tele));
}

void FujinonCamera::zoomWide()
{
    enqueueCommand(m_builder.buildZoom(ZoomDirection::Wide));
}

void FujinonCamera::zoomStop()
{
    enqueueCommand(m_builder.buildStop());
}

void FujinonCamera::focusNear()
{
    enqueueCommand(m_builder.buildFocus(FocusDirection::Near));
}

void FujinonCamera::focusFar()
{
    enqueueCommand(m_builder.buildFocus(FocusDirection::Far));
}

void FujinonCamera::focusStop()
{
    enqueueCommand(m_builder.buildStop());
}

void FujinonCamera::irisOpen()
{
    enqueueCommand(m_builder.buildIris(IrisDirection::Open));
}

void FujinonCamera::irisClose()
{
    enqueueCommand(m_builder.buildIris(IrisDirection::Close));
}

void FujinonCamera::irisStop()
{
    enqueueCommand(m_builder.buildStop());
}

void FujinonCamera::setZoomPosition(std::uint16_t pulse)
{
    enqueueCommand(m_builder.buildSetZoomPosition(pulse));
}

void FujinonCamera::setFocusPosition(std::uint16_t pulse)
{
    enqueueCommand(m_builder.buildSetFocusPosition(pulse));
}

void FujinonCamera::setIrisPosition(std::uint16_t pos)
{
    enqueueCommand(m_builder.buildSetIrisPosition(pos));
}

void FujinonCamera::setManualIris(ManualIrisFNo fNo)
{
    enqueueCommand(m_builder.buildSetManualIris(fNo));
}

void FujinonCamera::setShutterLimit(ShutterLimitMode limit)
{
    enqueueCommand(m_builder.buildSetShutterLimit(limit));
}

void FujinonCamera::setOnePushAf()
{
    enqueueCommand(m_builder.buildOnePushAf());
}

void FujinonCamera::setAutoFocus(AutoFocusMode mode)
{
    enqueueCommand(m_builder.buildSetAutoFocus(mode));
}

void FujinonCamera::setAfArea(AfArea area)
{
    enqueueCommand(m_builder.buildSetAfArea(area));
}

void FujinonCamera::setAfSensitivity(AfSensitivity sens)
{
    enqueueCommand(m_builder.buildSetAfSensitivity(sens));
}

void FujinonCamera::setDayNightMode(DayNightMode mode)
{
    enqueueCommand(m_builder.buildSetDayNight(mode));
}

void FujinonCamera::setAutoDayNight(DayNightMode mode)
{
    enqueueCommand(m_builder.buildSetAutoDayNight(mode));
}

void FujinonCamera::setDayNightTrigger(bool enable)
{
    enqueueCommand(m_builder.buildSetDayNightTrigger(enable));
}

void FujinonCamera::setVlcFilter(VlcFilterMode mode)
{
    enqueueCommand(m_builder.buildSetVlcFilter(mode));
}

void FujinonCamera::setDefog(DefogMode mode)
{
    enqueueCommand(m_builder.buildSetDefog(mode));
}

void FujinonCamera::setDeHeatHaze(DeHeatHazeMode mode)
{
    enqueueCommand(m_builder.buildSetDeHeatHaze(mode));
}

void FujinonCamera::setOpticalStabilization(OpticalStabilization mode)
{
    enqueueCommand(m_builder.buildSetOpticalStabilization(mode));
}

void FujinonCamera::setBrightness(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetBrightness(val));
}

void FujinonCamera::setContrast(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetContrast(val));
}

void FujinonCamera::setSaturation(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetSaturation(val));
}

void FujinonCamera::setSharpness(std::uint8_t val)
{
    enqueueCommand(m_builder.buildSetSharpness(val));
}

void FujinonCamera::setWhiteBalance(WhiteBalanceMode mode)
{
    enqueueCommand(m_builder.buildSetWhiteBalance(mode));
}

void FujinonCamera::setColorTemperature(std::uint16_t kelvin)
{
    enqueueCommand(m_builder.buildSetColorTemperature(kelvin));
}

void FujinonCamera::setDigitalZoom(DigitalZoomMode mode, std::uint8_t mag)
{
    enqueueCommand(m_builder.buildSetDigitalZoom(mode, mag));
}

void FujinonCamera::setNoiseReduction(NoiseReductionLevel level)
{
    enqueueCommand(m_builder.buildSetNoiseReduction(level));
}

void FujinonCamera::setOpticalFilterDay(OpticalFilter filter)
{
    enqueueCommand(m_builder.buildSetOpticalFilterDay(filter));
}

void FujinonCamera::setOpticalFilterNight(OpticalFilter filter)
{
    enqueueCommand(m_builder.buildSetOpticalFilterNight(filter));
}

void FujinonCamera::setIrWavelength(IrWavelength wl)
{
    enqueueCommand(m_builder.buildSetIrWavelength(wl));
}

void FujinonCamera::setTermination(bool enable)
{
    enqueueCommand(m_builder.buildSetTermination(enable));
}

void FujinonCamera::reboot()
{
    enqueueCommand(m_builder.buildReboot());
}

void FujinonCamera::factoryReset()
{
    enqueueCommand(m_builder.buildFactoryReset());
}

void FujinonCamera::setVideoDisplayMode(VideoDisplayMode mode)
{
    enqueueCommand(m_builder.buildSetVideoDisplayMode(mode));
}

void FujinonCamera::setVideoFormat(HdFormat format)
{
    enqueueCommand(m_builder.buildSetHdFormat(format));
}

void FujinonCamera::setOsdDatePosition(OsdPosition pos)
{
    enqueueCommand(m_builder.buildSetDateTimePosition(pos));
}

void FujinonCamera::setOsdTitlePosition(OsdPosition pos)
{
    enqueueCommand(m_builder.buildSetTitlePosition(pos));
}

void FujinonCamera::setOsdIdPosition(OsdPosition pos)
{
    enqueueCommand(m_builder.buildSetIdPosition(pos));
}

void FujinonCamera::setRtcTime(std::uint16_t year, std::uint8_t month, std::uint8_t day, std::uint8_t hour,
    std::uint8_t minute, std::uint8_t second)
{
    enqueueCommand(m_builder.buildSetClockYear(year));
    enqueueCommand(m_builder.buildSetClockDate(month, day));
    enqueueCommand(m_builder.buildSetClockTime(hour, minute));
    enqueueCommand(m_builder.buildSetClockSecond(second));
}

void FujinonCamera::setRtcTime(std::uint8_t year, std::uint8_t month, std::uint8_t day, std::uint8_t hour,
    std::uint8_t minute, std::uint8_t second)
{
    const auto fullYear = static_cast<std::uint16_t>((year < 100U) ? (2000U + year) : year);
    setRtcTime(fullYear, month, day, hour, minute, second);
}

void FujinonCamera::setSdPlayback(SdPlaybackControl ctrl)
{
    enqueueCommand(m_builder.buildPlayMovieSd(ctrl));
}

void FujinonCamera::setSdPlaybackSpeed(SdPlaybackMode mode)
{
    enqueueCommand(m_builder.buildSelectMoviePlayMode(mode));
}

void FujinonCamera::setSdMovieFile(std::uint16_t fileNo)
{
    enqueueCommand(m_builder.buildSetFirstMovieFile(fileNo));
}

void FujinonCamera::sendMenuKey(MenuKey key)
{
    enqueueCommand(m_builder.buildMenuKey(key));
}

void FujinonCamera::sendMenuDirection(MenuDirection dir)
{
    enqueueCommand(m_builder.buildMenuDirection(dir));
}

void FujinonCamera::setPreset(PresetAction action, std::uint8_t presetId)
{
    enqueueCommand(m_builder.buildPreset(action, presetId));
}

void FujinonCamera::setBaudRate(BaudRate rate)
{
    enqueueCommand(m_builder.buildSetBaudRate(rate));
}

void FujinonCamera::sendRawFrame(const std::vector<std::uint8_t>& frame)
{
    enqueueCommand(frame);
}

void FujinonCamera::sendQueryFrame(const std::vector<std::uint8_t>& frame, std::string queryTag)
{
    enqueueCommand(frame, std::move(queryTag));
}

void FujinonCamera::queryAll()
{
    enqueueCommand(m_builder.buildQuerySerialNumber(), "QuerySerial");
    enqueueCommand(m_builder.buildQueryFwVersion(), "QueryFw");
    enqueueCommand(m_builder.buildQueryLensStatus(), "QueryLens");
    enqueueCommand(m_builder.buildQueryZoomPosition(), "QueryZoom");
    enqueueCommand(m_builder.buildQueryFocusPosition(), "QueryFocus");
    enqueueCommand(m_builder.buildQueryPhotoSettings(), "QueryPhotoSettings");
    enqueueCommand(m_builder.buildQueryImageQuality(), "QueryImageQuality");
    enqueueCommand(m_builder.buildQueryDisplaySettings(), "QueryDisplaySettings");
    enqueueCommand(m_builder.buildQueryOperationSettings(), "QueryOperationSettings");
    enqueueCommand(m_builder.buildQueryFineSettings(), "QueryFineSettings");
    enqueueCommand(m_builder.buildQueryDayNightEx(), "QueryDayNightEx");
    enqueueCommand(m_builder.buildQuerySpeedEx(), "QuerySpeedEx");
    enqueueCommand(m_builder.buildQueryTemperature(), "QueryTemperature");
    enqueueCommand(m_builder.buildQueryManualIris(), "QueryManualIris");
}

void FujinonCamera::queryTemperature()
{
    enqueueCommand(m_builder.buildQueryTemperature(), "QueryTemperature");
}

void FujinonCamera::queryManualIris()
{
    enqueueCommand(m_builder.buildQueryManualIris(), "QueryManualIris");
}

std::uint64_t FujinonCamera::rxOverflowDrops() const noexcept
{
    return m_rxOverflowDrops.load(std::memory_order_relaxed);
}

} // namespace FujinonSX800

#include "MockCameraDevice.h"
#include "PelcoDFrame.h"

#include <glog/logging.h>

namespace FujinonSX800 {

MockCameraDevice::MockCameraDevice(std::uint8_t address) noexcept
    : m_address { address }
{
}

MockInternalState MockCameraDevice::getInternalState() const
{
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_state;
}

void MockCameraDevice::setInternalState(const MockInternalState& state)
{
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_state = state;
}

bool MockCameraDevice::open()
{
    m_open = true;
    LOG(INFO) << "MockCameraDevice opened (simulating camera address: "
              << static_cast<int>(m_address) << ")";
    StateChangedCallback cb;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        cb = m_stateCallback;
    }
    if (cb) {
        cb(TransportState::Connected, "Mock camera connected");
    }
    return true;
}

void MockCameraDevice::close()
{
    m_open = false;
    LOG(INFO) << "MockCameraDevice closed";
    StateChangedCallback cb;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        cb = m_stateCallback;
    }
    if (cb) {
        cb(TransportState::Disconnected, "Mock camera disconnected");
    }
}

bool MockCameraDevice::isOpen() const noexcept
{
    return m_open.load();
}

bool MockCameraDevice::sendData(const std::vector<std::uint8_t>& data)
{
    if (!m_open.load()) {
        return false;
    }

    VLOG(1) << "MockCameraDevice processing input frame (" << data.size() << " bytes)";

    const auto frames = PelcoDFrame::splitStream(data);
    for (const auto& frame : frames) {
        processIncomingFrame(frame);
    }
    return true;
}

void MockCameraDevice::setDataCallback(DataReceivedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_dataCallback = std::move(callback);
}

void MockCameraDevice::setStateCallback(StateChangedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_stateCallback = std::move(callback);
}

void MockCameraDevice::processIncomingFrame(const std::vector<std::uint8_t>& frame)
{
    if (frame.size() != PelcoDFrame::StandardFrameSize) {
        return;
    }

    const std::uint8_t addr { frame[1] };
    if (addr != m_address && addr != 0x00U) {
        return;
    }

    const std::uint8_t cmd1 { frame[2] };
    const std::uint8_t cmd2 { frame[3] };
    const std::uint8_t d1   { frame[4] };
    const std::uint8_t d2   { frame[5] };

    std::vector<std::uint8_t> response;

    // Standard Pelco-D query or command dispatch
    if (cmd1 == 0x00U && cmd2 == 0x55U) {
        // Query Zoom Position -> 0x00, 0x5D, MSB, LSB
        std::uint16_t zoom { 1000U };
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            zoom = m_state.zoomPosition;
        }
        const std::uint8_t msb = static_cast<std::uint8_t>((zoom >> 8U) & 0xFFU);
        const std::uint8_t lsb = static_cast<std::uint8_t>(zoom & 0xFFU);
        response = PelcoDFrame::createFrame(m_address, 0x00U, 0x5DU, msb, lsb);
    } else if (cmd1 == 0x00U && cmd2 == 0x73U) {
        // Query Focus Position -> 0x00, 0x81, MSB, LSB
        std::uint16_t focus { 2000U };
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            focus = m_state.focusPosition;
        }
        const std::uint8_t msb = static_cast<std::uint8_t>((focus >> 8U) & 0xFFU);
        const std::uint8_t lsb = static_cast<std::uint8_t>(focus & 0xFFU);
        response = PelcoDFrame::createFrame(m_address, 0x00U, 0x81U, msb, lsb);
    } else if (cmd1 == 0x00U && cmd2 == 0x77U) {
        // Query Shutter
        response = PelcoDFrame::createFrame(m_address, 0x00U, 0x85U, 0x00U, 0x05U);
    } else if (cmd1 == 0x00U && cmd2 == 0x79U) {
        // Query ISO
        response = PelcoDFrame::createFrame(m_address, 0x00U, 0x87U, 0x00U, 0x02U);
    } else if (cmd1 == 0x00U && cmd2 == 0x7BU) {
        // Query Serial Number -> 18-byte frame
        std::string serial;
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            serial = m_state.serialNumber;
        }
        response.assign(18U, 0x00U);
        response[0] = PelcoDFrame::SyncByte;
        response[1] = m_address;
        for (std::size_t i { 0U }; i < serial.size() && i < 15U; ++i) {
            response[2U + i] = static_cast<std::uint8_t>(serial[i]);
        }
        response[17] = PelcoDFrame::calculateChecksum(&response[1], 16U);
    } else if (cmd1 == 0x00U && cmd2 == 0x7DU) {
        // Query FW Version -> 0x00, 0x8B, Major, Minor
        std::uint8_t maj { 2U };
        std::uint8_t min { 12U };
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            maj = m_state.fwMajor;
            min = m_state.fwMinor;
        }
        response = PelcoDFrame::createFrame(m_address, 0x00U, 0x8BU, maj, min);
    } else if (cmd1 == 0x00U && cmd2 == 0x7FU) {
        // Query Lens Status -> 0x00, 0x8D, d1, d2
        response = PelcoDFrame::createFrame(m_address, 0x00U, 0x8DU, 0x00U, 0x00U);
    } else if (cmd1 == 0x00U && cmd2 == 0x4FU) {
        // Set Zoom Position
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_state.zoomPosition = static_cast<std::uint16_t>((d1 << 8U) | d2);
        }
        // Send ACK
        response = { PelcoDFrame::SyncByte, m_address, 0x00U, 0x00U };
        response[3] = PelcoDFrame::calculateChecksum(&response[1], 2U);
    } else if (cmd1 == 0x00U && cmd2 == 0x8FU) {
        // Set Focus Position
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_state.focusPosition = static_cast<std::uint16_t>((d1 << 8U) | d2);
        }
        response = { PelcoDFrame::SyncByte, m_address, 0x00U, 0x00U };
        response[3] = PelcoDFrame::calculateChecksum(&response[1], 2U);
    } else if (cmd1 == 0xF0U && cmd2 == 0x29U) {
        // Set Defog
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_state.defogMode = static_cast<DefogMode>(d2);
        }
        response = { PelcoDFrame::SyncByte, m_address, 0x00U, 0x00U };
        response[3] = PelcoDFrame::calculateChecksum(&response[1], 2U);
    } else if (cmd1 == 0xF0U && cmd2 == 0x27U) {
        // Set De-Heat Haze
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_state.deHeatHazeMode = static_cast<DeHeatHazeMode>(d2);
        }
        response = { PelcoDFrame::SyncByte, m_address, 0x00U, 0x00U };
        response[3] = PelcoDFrame::calculateChecksum(&response[1], 2U);
    } else if (cmd1 == 0xF0U && cmd2 == 0x21U) {
        // Set VLC filter
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_state.vlcFilterMode = static_cast<VlcFilterMode>(d2);
        }
        response = { PelcoDFrame::SyncByte, m_address, 0x00U, 0x00U };
        response[3] = PelcoDFrame::calculateChecksum(&response[1], 2U);
    } else {
        // General ACK for other commands
        response = { PelcoDFrame::SyncByte, m_address, 0x00U, 0x00U };
        response[3] = PelcoDFrame::calculateChecksum(&response[1], 2U);
    }

    if (!response.empty()) {
        DataReceivedCallback cb;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            cb = m_dataCallback;
        }
        if (cb) {
            cb(response);
        }
    }
}

} // namespace FujinonSX800

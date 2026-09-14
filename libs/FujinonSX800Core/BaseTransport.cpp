/// @file BaseTransport.cpp
/// @brief Shared transport callback and thread lifecycle implementation.

#include "BaseTransport.h"

#include <utility>

namespace FujinonSX800 {

void BaseTransport::setDataCallback(DataReceivedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_dataCallback = std::move(callback);
}

void BaseTransport::setStateCallback(StateChangedCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_stateCallback = std::move(callback);
}

void BaseTransport::notifyState(TransportState state, const std::string& message)
{
    StateChangedCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        callback = m_stateCallback;
    }
    if (callback) {
        callback(state, message);
    }
}

void BaseTransport::invokeDataCallback(const std::uint8_t* data, std::size_t size)
{
    DataReceivedCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        callback = m_dataCallback;
    }
    if (callback) {
        callback(data, size);
    }
}

void BaseTransport::stopReadThread()
{
    if (m_readThread.joinable()) {
        m_readThread.join();
    }
}

} // namespace FujinonSX800

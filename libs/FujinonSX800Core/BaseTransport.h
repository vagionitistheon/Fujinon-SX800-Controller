#pragma once

/// @file BaseTransport.h
/// @brief Shared callback, synchronization, and read-thread lifecycle support.

#include "ITransport.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace FujinonSX800 {

/// @class BaseTransport
/// @brief Common thread-safe implementation for streaming transports.
class BaseTransport : public ITransport {
public:
    BaseTransport() = default;
    ~BaseTransport() override = default;

    BaseTransport(const BaseTransport&) = delete;
    BaseTransport& operator=(const BaseTransport&) = delete;
    BaseTransport(BaseTransport&&) = delete;
    BaseTransport& operator=(BaseTransport&&) = delete;

    void setDataCallback(DataReceivedCallback callback) override;
    void setStateCallback(StateChangedCallback callback) override;

protected:
    void notifyState(TransportState state, const std::string& message);
    void invokeDataCallback(const std::uint8_t* data, std::size_t size);
    void stopReadThread();

    std::atomic<bool> m_running { false };
    std::thread m_readThread;
    mutable std::mutex m_callbackMutex;
    mutable std::mutex m_writeMutex;

private:
    DataReceivedCallback m_dataCallback;
    StateChangedCallback m_stateCallback;
};

} // namespace FujinonSX800

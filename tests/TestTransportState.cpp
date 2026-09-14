/// @file TestTransportState.cpp
/// @brief Unit tests for transport state propagation through FujinonCamera.

#include "FujinonSX800Core/FujinonCamera.h"
#include "TestHelper.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

namespace {

class StateTransport final : public FujinonSX800::ITransport {
public:
    [[nodiscard]] bool open() override
    {
        const auto remainingFailures = m_openFailures.load();
        if (remainingFailures > 0U) {
            m_openFailures.fetch_sub(1U);
            emitState(FujinonSX800::TransportState::Error, "open failure");
            return false;
        }

        m_open = true;
        m_openCount.fetch_add(1U);
        emitState(FujinonSX800::TransportState::Connected, "connected");
        return true;
    }

    void close() override
    {
        const bool wasOpen = m_open.exchange(false);
        if (wasOpen) {
            emitState(FujinonSX800::TransportState::Disconnected, "closed");
        }
    }

    [[nodiscard]] bool isOpen() const noexcept override
    {
        return m_open.load();
    }

    [[nodiscard]] bool sendData(const std::vector<std::uint8_t>&) override
    {
        return m_open.load();
    }

    void setDataCallback(DataReceivedCallback callback) override
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_dataCallback = std::move(callback);
    }

    void setStateCallback(StateChangedCallback callback) override
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_stateCallback = std::move(callback);
    }

    void emitState(FujinonSX800::TransportState state, const std::string& message)
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

    void failNextOpens(std::uint32_t count)
    {
        m_openFailures = count;
    }

    [[nodiscard]] std::uint32_t openCount() const noexcept
    {
        return m_openCount.load();
    }

private:
    std::atomic<bool> m_open { false };
    std::atomic<std::uint32_t> m_openFailures { 0U };
    std::atomic<std::uint32_t> m_openCount { 0U };
    std::mutex m_callbackMutex;
    DataReceivedCallback m_dataCallback;
    StateChangedCallback m_stateCallback;
};

void testStatePropagation()
{
    auto transport = std::make_shared<StateTransport>();
    FujinonSX800::FujinonCamera camera(transport, 0x07U);

    std::atomic<FujinonSX800::TransportState> lastState { FujinonSX800::TransportState::Disconnected };
    camera.addTransportStateCallback(
        [&](FujinonSX800::TransportState state, const std::string&) { lastState = state; });

    SX800_TEST_ASSERT(camera.start());
    SX800_TEST_ASSERT(lastState.load() == FujinonSX800::TransportState::Connected);
    SX800_TEST_ASSERT(camera.getStatus().isConnected);

    transport->emitState(FujinonSX800::TransportState::Error, "link failure");
    SX800_TEST_ASSERT(lastState.load() == FujinonSX800::TransportState::Error);
    SX800_TEST_ASSERT(!camera.getStatus().isConnected);

    camera.stop();
}

void testAutoReconnect()
{
    auto transport = std::make_shared<StateTransport>();
    FujinonSX800::FujinonCamera camera(transport, 0x07U);
    camera.setAutoReconnect(true);

    SX800_TEST_ASSERT(camera.start());
    transport->failNextOpens(1U);
    transport->emitState(FujinonSX800::TransportState::Error, "link failure");

    bool reconnected { false };
    for (std::uint32_t attempt { 0U }; attempt < 40U; ++attempt) {
        if (transport->openCount() >= 2U && camera.isConnected()) {
            reconnected = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    SX800_TEST_ASSERT(reconnected);
    camera.stop();
}

} // namespace

int main()
{
    testStatePropagation();
    testAutoReconnect();
    std::cout << "[PASS] TestTransportState completed successfully." << std::endl;
    return 0;
}

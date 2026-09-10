/// @file TestMockCamera.cpp
/// @brief Integration test of FujinonCamera controller running against MockCameraDevice.

#include "FujinonSX800Core/FujinonCamera.h"
#include "FujinonSX800Core/MockCameraDevice.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

void testMockIntegration()
{
    auto mockDevice = std::make_shared<FujinonSX800::MockCameraDevice>(0x07U);
    FujinonSX800::FujinonCamera camera(mockDevice, 0x07U);

    std::atomic<bool> gotStatusUpdate { false };
    camera.addStatusCallback([&](const FujinonSX800::CameraStatus& status) {
        if (!status.serialNumber.empty() && status.fwVersionMajor == 2U) {
            gotStatusUpdate = true;
        }
    });

    camera.setAutoQueryOnConnect(true);
    assert(camera.start());
    assert(camera.isConnected());

    // Allow worker to query initial status
    for (int i = 0; i < 50; ++i) {
        if (gotStatusUpdate) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    assert(gotStatusUpdate);
    const auto status = camera.getStatus();
    assert(status.serialNumber == "SX800999");
    assert(status.fwVersionMajor == 2U && status.fwVersionMinor == 12U);
    assert(status.rs485Address == 0x07U);

    // Test commanding zoom position
    camera.setZoomPosition(12345U);
    bool zoomApplied { false };
    for (int i { 0 }; i < 50; ++i) {
        if (mockDevice->getInternalState().zoomPosition == 12345U) {
            zoomApplied = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    assert(zoomApplied);

    // Test setting defog mode
    camera.setDefog(FujinonSX800::DefogMode::High);
    bool defogApplied { false };
    for (int i { 0 }; i < 50; ++i) {
        if (mockDevice->getInternalState().defogMode == FujinonSX800::DefogMode::High) {
            defogApplied = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    assert(defogApplied);

    // Test query response timeout
    std::atomic<bool> timeoutTriggered { false };
    std::string timedOutTag;
    camera.setQueryTimeoutMs(50U);
    camera.addTimeoutCallback([&](const std::string& tag) {
        timeoutTriggered = true;
        timedOutTag = tag;
    });

    // Send an unhandled command with a query tag to trigger timeout (address 0x09 is ignored by mock device)
    camera.sendQueryFrame(FujinonSX800::PelcoDFrame::createFrame(0x09U, 0x00U, 0x55U, 0x00U, 0x00U), "TestQuery");
    for (int i = 0; i < 40; ++i) {
        if (timeoutTriggered.load()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert(timeoutTriggered.load());
    assert(timedOutTag == "TestQuery");

    camera.stop();
    assert(!camera.isConnected());
}

#include <glog/logging.h>

int main([[maybe_unused]] int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;
    FLAGS_colorlogtostderr = true;

    testMockIntegration();

    std::cout << "[PASS] TestMockCamera completed successfully." << std::endl;
    google::ShutdownGoogleLogging();
    return 0;
}

/// @file TestConnectionWidget.cpp
/// @brief Qt widget tests for connection state presentation and user interactions.

#include "ConnectionWidget.h"
#include "TestHelper.h"

#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QMetaType>
#include <QPushButton>
#include <QSignalSpy>
#include <QStackedWidget>

#include <iostream>
#include <memory>
#include <string>

using FujinonSX800App::ConnectionWidget;

namespace {

bool containsLabel(const ConnectionWidget& widget, const QString& text)
{
    const auto labels = widget.findChildren<QLabel*>();
    for (const auto* label : labels) {
        if (label->text().contains(text)) {
            return true;
        }
    }
    return false;
}

QPushButton* findButtonByText(const ConnectionWidget& widget, const QString& text)
{
    const auto buttons = widget.findChildren<QPushButton*>();
    for (auto* button : buttons) {
        if (button->text() == text) {
            return button;
        }
    }
    return nullptr;
}

void testConnectionStatePresentation()
{
    ConnectionWidget widget;
    widget.setConnectionState(true);
    SX800_TEST_ASSERT(containsLabel(widget, "Online"));

    widget.setTransportState(FujinonSX800::TransportState::Disconnected, "socket closed");
    SX800_TEST_ASSERT(containsLabel(widget, "Reconnecting"));

    widget.setTransportState(FujinonSX800::TransportState::Error, "link failure");
    SX800_TEST_ASSERT(containsLabel(widget, "link failure"));

    const auto buttons = widget.findChildren<QPushButton*>();
    bool hasConnectButton { false };
    for (const auto* button : buttons) {
        if (button->text() == "Connect" || button->text() == "Cancel") {
            hasConnectButton = true;
        }
    }
    SX800_TEST_ASSERT(hasConnectButton);
}

void testModeSwitching()
{
    ConnectionWidget widget;
    const auto combos = widget.findChildren<QComboBox*>();
    SX800_TEST_ASSERT(!combos.empty());
    auto* cmbMode = combos.at(0);

    auto* stacked = widget.findChild<QStackedWidget*>();
    SX800_TEST_ASSERT(stacked != nullptr);

    // Modes: 0=Mock, 1=Serial, 2=TCP, 3=UDP
    for (int modeIndex { 0 }; modeIndex < 4; ++modeIndex) {
        cmbMode->setCurrentIndex(modeIndex);
        SX800_TEST_ASSERT(stacked->currentIndex() == modeIndex);
    }
}

void testSignalEmissions()
{
    qRegisterMetaType<std::shared_ptr<FujinonSX800::ITransport>>("std::shared_ptr<FujinonSX800::ITransport>");
    qRegisterMetaType<std::uint8_t>("std::uint8_t");

    ConnectionWidget widget;
    QSignalSpy connectSpy(&widget, &ConnectionWidget::connectRequested);
    QSignalSpy disconnectSpy(&widget, &ConnectionWidget::disconnectRequested);
    QSignalSpy scanSpy(&widget, &ConnectionWidget::scanBusRequested);

    // Default mode is Mock (index 0)
    auto* btnConnect = findButtonByText(widget, "Connect");
    SX800_TEST_ASSERT(btnConnect != nullptr);

    // 1. Click connect -> emits connectRequested
    btnConnect->click();
    SX800_TEST_ASSERT(connectSpy.count() == 1);

    // 2. Transition to connected state -> button becomes Disconnect
    widget.setConnectionState(true);
    auto* btnDisconnect = findButtonByText(widget, "Disconnect");
    SX800_TEST_ASSERT(btnDisconnect != nullptr);

    btnDisconnect->click();
    SX800_TEST_ASSERT(disconnectSpy.count() == 1);

    // 3. Disconnect state -> click Scan Bus
    widget.setConnectionState(false);
    auto* btnScan = findButtonByText(widget, "Scan Bus");
    SX800_TEST_ASSERT(btnScan != nullptr);

    btnScan->click();
    SX800_TEST_ASSERT(scanSpy.count() == 1);
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    testConnectionStatePresentation();
    testModeSwitching();
    testSignalEmissions();
    std::cout << "[PASS] TestConnectionWidget completed successfully." << std::endl;
    return 0;
}

/// @file main.cpp
/// @brief CLI entrypoint for the Fujinon SX800 Terminal User Interface (TUI) client.
/// @author Theon Sensors / Maintainers

#include "TuiApp.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void printUsage(const char* progName)
{
    std::cout << "Fujinon SX800 Pelco-D Controller - Zero-Dependency UTF-8 TUI Client\n\n"
              << "Usage:\n"
              << "  " << progName << " [options]\n\n"
              << "Options:\n"
              << "  -m, --mock                    Launch immediately using internal Mock SX800 Camera (default)\n"
              << "  -s, --serial <port> [baud]    Connect via Serial/RS-485 port (e.g. /dev/ttyUSB0 9600)\n"
              << "  -t, --tcp <host> <port>       Connect via TCP/IP socket bridge (e.g. 192.168.1.100 5000)\n"
              << "  -a, --address <addr>          Target RS-485 device address [1-255] (default: 7)\n"
              << "  -h, --help                    Display this help message and exit\n\n"
              << "Interactive Keybindings:\n"
              << "  [1-7]                         Switch tabs (Optics, ImgQ, Day/Night, OSD, SD, System, Traffic)\n"
              << "  [Tab] / [Shift-Tab]           Cycle through tabs\n"
              << "  [Z] / [X]                     Zoom In (Tele) / Zoom Out (Wide)\n"
              << "  [F] / [G]                     Focus Near / Focus Far\n"
              << "  [Space]                       Stop all active lens motion\n"
              << "  [A]                           Trigger One-Push AutoFocus\n"
              << "  [O]                           Toggle Optical Image Stabilization (OIS)\n"
              << "  [D]                           Toggle Day / Night mode\n"
              << "  [I]                           Toggle Auto Iris\n"
              << "  [R]                           Query all camera status registers\n"
              << "  [C]                           Open Connection Manager (Serial / TCP / Mock)\n"
              << "  [K]                           Clear traffic sniffer log\n"
              << "  [M]                           Switch / reconnect to internal mock simulation\n"
              << "  [Q]                           Quit application and restore terminal\n\n";
}

} // namespace

int main(int argc, char* argv[])
{
    bool useMock { true };
    std::string serialPort {};
    std::uint32_t baudRate { 9600U };
    std::string tcpHost {};
    std::uint16_t tcpPort { 0U };
    std::uint8_t address { 7U };

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        if (arg == "-m" || arg == "--mock") {
            useMock = true;
        } else if ((arg == "-s" || arg == "--serial") && i + 1 < argc) {
            useMock = false;
            serialPort = argv[++i];
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                baudRate = static_cast<std::uint32_t>(std::strtoul(argv[++i], nullptr, 10));
            }
        } else if ((arg == "-t" || arg == "--tcp") && i + 2 < argc) {
            useMock = false;
            tcpHost = argv[++i];
            tcpPort = static_cast<std::uint16_t>(std::strtoul(argv[++i], nullptr, 10));
        } else if ((arg == "-a" || arg == "--address") && i + 1 < argc) {
            address = static_cast<std::uint8_t>(std::strtoul(argv[++i], nullptr, 10));
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    FujinonSX800Tui::TuiApp app {};

    if (useMock && serialPort.empty() && tcpHost.empty()) {
        if (!app.initMock(address)) {
            std::cerr << "Failed to initialize virtual mock camera\n";
            return 1;
        }
    } else if (!serialPort.empty()) {
        if (!app.initSerial(serialPort, baudRate, address)) {
            std::cerr << "Failed to initialize serial connection on " << serialPort << "\n";
            return 1;
        }
    } else if (!tcpHost.empty()) {
        if (!app.initTcp(tcpHost, tcpPort, address)) {
            std::cerr << "Failed to initialize TCP connection to " << tcpHost << ":" << tcpPort << "\n";
            return 1;
        }
    } else {
        if (!app.initMock(address)) {
            std::cerr << "Failed to initialize virtual mock camera\n";
            return 1;
        }
    }

    return app.run();
}

/// @file main.cpp
/// @brief CLI entrypoint for the Fujinon SX800 Terminal User Interface (TUI) client.
/// @author Theon Sensors / Maintainers

#include "FujinonSX800Core/BusScanner.h"
#include "FujinonSX800Core/SerialTransport.h"
#include "FujinonSX800Core/TcpTransport.h"
#include "FujinonSX800Core/UdpTransport.h"
#include "TuiApp.h"

#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
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
              << "  -u, --udp <host> <port> [local] Connect via UDP socket bridge\n"
              << "      --scan                    Scan the selected transport for Pelco-D addresses and exit\n"
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
              << "  [C]                           Open Connection Manager (Serial / TCP / UDP / Mock)\n"
              << "  [K]                           Clear traffic sniffer log\n"
              << "  [M]                           Switch / reconnect to internal mock simulation\n"
              << "  [Q]                           Quit application and restore terminal\n\n";
}

} // namespace

int main(int argc, char* argv[])
{
    bool useMock { true };
    bool scanBus { false };
    std::string serialPort {};
    std::uint32_t baudRate { 9600U };
    std::string tcpHost {};
    std::uint16_t tcpPort { 0U };
    std::string udpHost {};
    std::uint16_t udpPort { 0U };
    std::uint16_t udpLocalPort { 0U };
    std::uint8_t address { 7U };

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        if (arg == "--scan") {
            scanBus = true;
            continue;
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
        } else if ((arg == "-u" || arg == "--udp") && i + 2 < argc) {
            useMock = false;
            udpHost = argv[++i];
            udpPort = static_cast<std::uint16_t>(std::strtoul(argv[++i], nullptr, 10));
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                udpLocalPort = static_cast<std::uint16_t>(std::strtoul(argv[++i], nullptr, 10));
            }
        } else if ((arg == "-a" || arg == "--address") && i + 1 < argc) {
            address = static_cast<std::uint8_t>(std::strtoul(argv[++i], nullptr, 10));
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (scanBus) {
        std::shared_ptr<FujinonSX800::ITransport> transport;
        if (!serialPort.empty()) {
            transport = std::make_shared<FujinonSX800::SerialTransport>(serialPort, baudRate);
        } else if (!tcpHost.empty()) {
            transport = std::make_shared<FujinonSX800::TcpTransport>(tcpHost, tcpPort);
        } else if (!udpHost.empty()) {
            transport = std::make_shared<FujinonSX800::UdpTransport>(udpHost, udpPort, udpLocalPort);
        } else {
            transport = std::make_shared<FujinonSX800::MockCameraDevice>(address);
        }

        FujinonSX800::BusScanner scanner(transport);
        std::mutex scanMutex;
        std::condition_variable scanCv;
        bool scanFinished { false };
        scanner.setDeviceDiscoveredCallback([](const FujinonSX800::DiscoveredDevice& device) {
            std::cout << "Found Pelco-D device at address " << static_cast<int>(device.address) << " ("
                      << device.responseTimeMs << " ms)\n";
        });
        scanner.setScanProgressCallback([](std::uint8_t current, std::size_t scanned, std::size_t total) {
            std::cout << "Scanned address " << static_cast<int>(current) << " (" << scanned << "/" << total << ")\n";
        });
        scanner.setScanFinishedCallback([&](const std::vector<FujinonSX800::DiscoveredDevice>&) {
            std::lock_guard<std::mutex> lock(scanMutex);
            scanFinished = true;
            scanCv.notify_one();
        });

        if (!scanner.startScan()) {
            std::cerr << "Failed to start Pelco-D bus scan\n";
            return 1;
        }
        std::unique_lock<std::mutex> lock(scanMutex);
        scanCv.wait(lock, [&] { return scanFinished; });
        return 0;
    }

    FujinonSX800Tui::TuiApp app {};

    if (useMock && serialPort.empty() && tcpHost.empty() && udpHost.empty()) {
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
    } else if (!udpHost.empty()) {
        if (!app.initUdp(udpHost, udpPort, udpLocalPort, address)) {
            std::cerr << "Failed to initialize UDP connection to " << udpHost << ":" << udpPort << "\n";
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

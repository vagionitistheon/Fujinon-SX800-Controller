# Fujinon SX800 / SX801 Camera Controller

Cross-platform C++17 controller and Qt 6 GUI application for the **Fujinon SX800 / SX801** long-range multi-sensor surveillance camera over the full Pelco-D protocol specification (v2.12.0).

---

## Architecture

```text
+-------------------------------------------------------------------------------+
|                           FujinonSX800App (Qt 6 GUI)                          |
|  - Modern Dark Dashboard                                                      |
|  - Tabs: Optics/PTZ, Image Quality, Day/Night, OSD/Video, SD Card, System     |
|  - Real-Time Protocol Traffic Inspector (TX/RX Hex & Timestamped Log)         |
+---------------------------------------+---------------------------------------+
                                        |
                                        v
+-------------------------------------------------------------------------------+
|                       FujinonSX800Qt (Qt 6 Adapter Layer)                     |
|  - QFujinonCamera (QObject wrapper with signals, slots, and QThread safety)   |
+---------------------------------------+---------------------------------------+
                                        |
                                        v
+-------------------------------------------------------------------------------+
|                     FujinonSX800Core (Standalone C++17 Library)               |
|  - Strict Zero-Qt Dependency (usable by CLI, Dear ImGui, ROS, embedded)       |
|  - PelcoDFrame: 4-byte general, 7-byte standard, 18-byte extended framing     |
|  - ProtocolBuilder: 100% command coverage (5.1 - 5.13)                        |
|  - ProtocolParser: Status & telemetry response decoder                        |
|  - CircularByteRing: SPSC lock-free circular byte stream ring                 |
|  - Transports: Cross-platform native Serial (RS-485) and TCP Sockets          |
|  - MockCameraDevice: In-memory simulation engine for hardware-free CI testing |
+-------------------------------------------------------------------------------+
```

---

## Key Features

- **Full Pelco-D Protocol Coverage (v2.12.0)**:
  - Optics & PTZ (continuous/step zoom, focus near/far, one-push AF, iris auto/manual).
  - Optical lookup conversions: Zoom pulse to focal length / horizontal FOV, Focus pulse to object distance.
  - Image processing: Optical Image Stabilization (OIS), Defog (3 levels), De-Heat Haze, Visible Light Cut (VLC).
  - Day/Night switching, optical filters (Clear, IR-Pass, IR Cut), and IR wavelength selection.
  - OSD menus, digital zoom (1.0x–4.0x), white balance, noise reduction, and video output standards (NTSC/PAL).
  - SD card playback controls and virtual remote D-Pad navigation.
- **Lock-Free RX Streaming**:
  - Transport I/O read threads push raw bytes into an SPSC `CircularByteRing` without acquiring mutexes or allocating dynamic memory.
  - Decoupled consumer thread reassembles Pelco-D packets across fragmented TCP/serial chunk boundaries.
- **Cross-Platform Transports**:
  - **Serial/RS-485**: POSIX `termios` on Linux, Win32 Communications API on Windows (`CreateFileA`, `SetCommState`, `SetCommTimeouts`).
  - **TCP Sockets**: POSIX BSD sockets on Linux, Winsock2 (`WSAStartup`) on Windows with non-blocking connect timeouts.
- **Hardware-Free Testing**:
  - Includes `MockCameraDevice` simulating Fujinon SX800 register responses and state transitions for offline testing and CI.

---

## Directory Structure

```text
Fujinon-SX800-Controller/
├── .agents/                      # Architecture, compliance, and coding rules
├── CMakeLists.txt                # Unified multi-target CMake build script
├── docs/                         # Protocol specs and design documentation
├── libs/
│   ├── FujinonSX800Core/         # Pure C++17 core library (no Qt dependency)
│   │   ├── CameraStatus.h        # Unified camera telemetry state struct
│   │   ├── CircularByteRing.h    # SPSC lock-free circular byte stream ring
│   │   ├── FujinonCamera.h/.cpp  # Asynchronous camera controller & command pacing
│   │   ├── ITransport.h          # Abstract transport interface
│   │   ├── MockCameraDevice.h/.cpp# In-memory virtual camera simulation
│   │   ├── OpticalTables.h/.cpp  # Appendix 1 & 2 focal/distance lookup tables
│   │   ├── PelcoDFrame.h/.cpp    # Framing, checksums, and stream splitting
│   │   ├── PelcoDTypes.h         # Command constants, registers, enums
│   │   ├── ProtocolBuilder.h/.cpp# Pelco-D command frame factory
│   │   ├── ProtocolParser.h/.cpp # Telemetry response parser
│   │   ├── SerialTransport.h/.cpp# Linux/Windows serial port driver
│   │   └── TcpTransport.h/.cpp   # Linux/Windows TCP socket driver
│   └── FujinonSX800Qt/           # Qt 6 adapter layer
│       ├── QFujinonCamera.h      # QObject wrapper with signals/slots
│       └── QFujinonCamera.cpp
├── app-qt/                       # Qt 6 desktop application
│   ├── main.cpp
│   ├── MainWindow.h/.cpp         # Main dashboard window
│   ├── resources.qrc             # Application resources
│   ├── tabs/                     # Control tabs (Optics, Quality, DayNight, etc.)
│   └── widgets/                  # Connection bar and Traffic Inspector
└── tests/                        # Automated unit and concurrency test suites
    ├── TestCircularByteRing.cpp  # SPSC lock-free throughput and concurrency tests
    ├── TestMockCamera.cpp        # Full round-trip simulation tests
    ├── TestOpticalTables.cpp     # Focal length and distance lookup validation
    ├── TestPelcoDFrame.cpp       # Framing and checksum calculation tests
    ├── TestProtocolBuilder.cpp   # Command frame generation tests
    └── TestProtocolParser.cpp    # Telemetry and response parsing tests
```

---

## Prerequisites

- **Compiler**: C++17 compliant compiler (GCC 9+, Clang 10+, or MSVC 2019+).
- **Build System**: CMake 3.16+.
- **GUI & Qt Adapter**: Qt 6.2+ (`Core`, `Gui`, `Widgets`, `Test`).
  - *Note: `FujinonSX800Core` has zero Qt dependency and can be compiled independently.*

---

## Build Instructions

### Linux

```bash
# Clone the repository
git clone https://github.com/your-org/Fujinon-SX800-Controller.git
cd Fujinon-SX800-Controller

# Configure build with Qt 6
cmake -B build -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64

# Build all targets (Core, Qt Adapter, App, Tests)
cmake --build build -j$(nproc)
```

### Windows (MSVC)

```cmd
cmake -B build -S . -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64
cmake --build build --config Release
```

---

## Running the Application

Launch the built executable from the build directory:

```bash
./build/FujinonSX800App
```

1. **Connection**:
   - **Serial Mode**: Select `/dev/ttyUSB*` (Linux) or `COM*` (Windows) and set baud rate (default: `9600` or `115200`).
   - **TCP Mode**: Enter camera IP (default: `192.168.0.10`) and port (`4001`).
2. **Offline Simulation**:
   - Pass an in-memory `MockCameraDevice` transport programmatically to test the full GUI without physical hardware.

---

## Running Automated Tests

All tests are integrated with CTest:

```bash
ctest --test-dir build --output-on-failure
```

### Test Suites

| Test Target | Description |
|---|---|
| `TestPelcoDFrame` | Validates standard (7B), query (18B), general (4B) frames & 8-bit checksums |
| `TestProtocolBuilder` | Tests command construction across all 13 Pelco-D functional groups |
| `TestProtocolParser` | Validates response parsing, telemetry extraction, and register decoding |
| `TestOpticalTables` | Verifies interpolation for focal length, horizontal FOV, and focus distance |
| `TestMockCamera` | Tests bidirectional command handling and state updates via simulated device |
| `TestCircularByteRing` | Tests SPSC lock-free wrap-around, sync byte search, and multi-threaded stress |

---

## License

Proprietary / Internal Use. See repository licensing terms.

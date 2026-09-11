# C4 Model

The Fujinon SX800 Controller communicates with Fujinon SX800/SX801 cameras over
Pelco-D using either a serial RS-485 connection or TCP. The Qt and terminal
applications share the zero-Qt core library.

## System Context

### ASCII

```text
                                  +----------------------+
                                  | Camera Operator      |
                                  | Uses the controller  |
                                  +----------+-----------+
                                             |
                                             | operates
                                             v
                         +-------------------+-------------------+
                         | Fujinon SX800 Controller             |
                         | Qt dashboard and terminal interface  |
                         +-------------------+-------------------+
                                             |
                                             | Pelco-D over RS-485 or TCP
                                             v
                                  +----------+-----------+
                                  | Fujinon SX800/SX801  |
                                  | Multi-sensor camera  |
                                  +----------------------+
```

### Mermaid

```mermaid
C4Context
    title System Context - Fujinon SX800 Controller

    Person(operator, "Camera Operator", "Operates the camera controller")
    System(controller, "Fujinon SX800 Controller", "Qt dashboard and terminal interface")
    System_Ext(camera, "Fujinon SX800/SX801", "Multi-sensor camera controlled with Pelco-D")

    Rel(operator, controller, "Operates")
    Rel(controller, camera, "Controls and monitors\nPelco-D over RS-485 or TCP")
```

## Containers

### ASCII

```text
+----------------------+       +---------------------------+
| Camera Operator      |       | Fujinon SX800/SX801       |
+----------+-----------+       | Multi-sensor camera       |
           |                   +-------------^-------------+
           | operates                        |
           v                                  | Pelco-D over RS-485/TCP
+----------+-----------+                      |
| Fujinon SX800        +----------------------+
| Controller            |
+----------+-----------+
           |
           | uses
           +----------------------+----------------------+
           |                     |                      |
           v                     v                      v
+----------+-----------+ +-------+----------------+ +---+----------------+
| Qt GUI application   | | Terminal UI           | | Automated tests  |
| Qt 6 dashboard       | | Terminal controller   | | Core behavior    |
+----------+-----------+ +-------+----------------+ +---+----------------+
           |                     |                      |
           | uses                | uses                 | verifies
           v                     v                      v
+----------+----------------------------------------------+
| FujinonSX800Core                                      |
| C++17 protocol, camera state, parsing, and transports |
+----------------------+--------------------------------+
                       ^
                       |
                       | adapts core API to Qt
                       |
              +--------+---------+
              | FujinonSX800Qt   |
              | Qt QObject layer |
              +------------------+
```

### Mermaid

```mermaid
C4Container
    title Container Diagram - Fujinon SX800 Controller

    Person(operator, "Camera Operator", "Operates the camera controller")
    System_Ext(camera, "Fujinon SX800/SX801", "Multi-sensor camera")

    Container_Boundary(controller, "Fujinon SX800 Controller") {
        Container(qtApp, "Qt GUI Application", "Qt 6", "Dashboard for camera control and traffic inspection")
        Container(tuiApp, "Terminal UI", "C++17/TUI", "Terminal-based camera control interface")
        Container(core, "FujinonSX800Core", "C++17 library", "Protocol frames, camera state, parsing, and transports")
        Container(qtAdapter, "FujinonSX800Qt", "Qt 6 adapter", "QObject wrapper exposing the core to the Qt application")
        Container(tests, "Automated Tests", "CTest", "Verifies protocol, transport, lookup, and simulation behavior")
    }

    Rel(operator, qtApp, "Uses")
    Rel(operator, tuiApp, "Uses")
    Rel(qtApp, qtAdapter, "Uses")
    Rel(tuiApp, core, "Uses")
    Rel(qtAdapter, core, "Adapts")
    Rel(tests, core, "Verifies")
    Rel(core, camera, "Controls and monitors\nPelco-D over RS-485 or TCP")
```

## Components

The Component view zooms into the `FujinonSX800Core` container.

### ASCII

```text
+----------------------- FujinonSX800Core ------------------------+
|                                                                  |
|  +----------------+       +-------------------+                 |
|  | FujinonCamera  |------>| ProtocolBuilder   |                 |
|  | Controller     |       | Command serializer |                 |
|  +-------+--------+       +---------+---------+                 |
|          |                          |                            |
|          | parses                   | builds                     |
|          v                          v                            |
|  +-------+--------+       +---------+---------+                 |
|  | ProtocolParser |       | PelcoDFrame       |                 |
|  | Telemetry      |       | Framing/checksum  |                 |
|  +-------+--------+       +---------+---------+                 |
|          |                          |                            |
|          v                          v                            |
|  +-------+--------+       +---------+---------+                 |
|  | CameraStatus   |       | ITransport        |<-- Serial/TCP   |
|  | Telemetry model|       | Raw byte channel  |<-- Mock         |
|  +----------------+       +-------------------+                 |
|                                                                  |
|  CircularByteRing buffers fragmented receive data for the        |
|  controller's RX processing path.                                |
+------------------------------------------------------------------+
```

### Mermaid

```mermaid
C4Component
    title Component Diagram - FujinonSX800Core

    Container_Boundary(core, "FujinonSX800Core") {
        Component(camera, "FujinonCamera", "C++17 class", "Coordinates commands, receive processing, polling, and callbacks")
        Component(builder, "ProtocolBuilder", "C++17 class", "Serializes camera operations into Pelco-D frames")
        Component(parser, "ProtocolParser", "C++17 class", "Decodes responses into CameraStatus telemetry")
        Component(frame, "PelcoDFrame", "C++17 class", "Validates, frames, and splits Pelco-D packets")
        Component(transport, "ITransport", "C++17 interface", "Streams raw bytes and reports channel state")
        Component(ring, "CircularByteRing", "C++17 template", "Buffers fragmented receive data without allocation")
        Component(status, "CameraStatus", "C++17 struct", "Stores current camera telemetry and state")
    }

    Rel(camera, builder, "Builds commands with")
    Rel(camera, parser, "Parses received frames with")
    Rel(camera, frame, "Validates and splits with")
    Rel(camera, transport, "Reads and writes through")
    Rel(camera, ring, "Buffers received bytes in")
    Rel(parser, status, "Updates")
    Rel(builder, frame, "Creates")
```

## Code

The Code view shows the principal classes involved in one command and response
cycle. Mermaid uses a `classDiagram` here because it has no `C4Code` diagram
type.

### ASCII

```text
                     +----------------------+
                     | FujinonCamera        |
                     | start/stop           |
                     | command methods      |
                     | worker/rx/poll loops |
                     +----+------+------+---+
                          |      |      |
                 owns     |      |      | owns
                          |      |      v
                          |      |  +----------------+
                          |      +->| CircularByteRing|
                          |         | receive buffer |
                          |         +----------------+
                          |
              +-----------+-----------+----------------+
              |                       |                |
              v                       v                v
     +----------------+     +----------------+  +-------------+
     | ProtocolBuilder|     | ProtocolParser |  | ITransport  |
     | build commands |     | parse replies  |  | raw I/O     |
     +--------+-------+     +--------+-------+  +------+------+
              |                       |                |
              v                       v                +--+--+--+
       +--------------+       +--------------+             |  |  |
       | PelcoDFrame  |       | CameraStatus |         Serial TCP Mock
       | bytes/checks |       | telemetry    |
       +--------------+       +--------------+
```

### Mermaid

```mermaid
classDiagram
    class FujinonCamera {
        +start() bool
        +stop() void
        +setZoomPosition(pulse) void
        +queryAll() void
        +sendRawFrame(frame) void
        -workerLoop() void
        -rxLoop() void
        -pollingLoop() void
    }

    class ProtocolBuilder {
        +buildZoom(direction) vector~uint8_t~
        +buildSetDayNight(mode) vector~uint8_t~
        +buildQueryImageQuality() vector~uint8_t~
    }

    class ProtocolParser {
        +parsePacket(packet, status, lastQuery) bool
        +describeFrame(frame, isTx) string
    }

    class PelcoDFrame {
        +encode() vector~uint8_t~
        +decode(packet) PelcoDFrame
        +isValid(packet) bool
    }

    class ITransport {
        <<interface>>
        +open() bool
        +close() void
        +sendData(data) bool
        +setDataCallback(callback) void
    }

    class SerialTransport
    class TcpTransport
    class MockCameraDevice

    class CircularByteRing {
        +push(data) bool
        +pop() uint8_t
        +available() size_t
    }

    class CameraStatus {
        +zoomPosition
        +focusPosition
        +dayNightMode
        +temperature
    }

    FujinonCamera *-- ProtocolBuilder : owns
    FujinonCamera *-- CircularByteRing : owns
    FujinonCamera --> ITransport : uses
    FujinonCamera --> ProtocolParser : invokes
    FujinonCamera --> CameraStatus : updates
    ProtocolBuilder --> PelcoDFrame : creates
    ProtocolParser --> PelcoDFrame : validates
    ProtocolParser --> CameraStatus : fills
    ITransport <|.. SerialTransport
    ITransport <|.. TcpTransport
    ITransport <|.. MockCameraDevice
```

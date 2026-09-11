---
name: doxygen-docs
description: >-
  Use this skill when documenting C++ code, classes, structs, methods, or free functions with Doxygen templates for the Fujinon SX800 Controller.
---

# Doxygen Documentation Standards for Fujinon SX800 Controller

Every class, struct, interface, method, enum, and free function must be fully documented using Doxygen structural tags before release. Use the `///` triple-slash comment style throughout.

---

## 1. File Header Template

Every header and implementation file must begin with a descriptive `@file` block:

```cpp
/// @file FujinonCamera.h
/// @brief High-level controller interface for the Fujinon SX800 camera.
/// @details Coordinates transport I/O, asynchronous response parsing, pacing queues,
///          and optical telemetry according to the Fujinon SX800 Pelco-D Specification (v2.12.0).
/// @author Theon Sensors / Maintainers
```

---

## 2. Class & Struct Template

Document class responsibility, layer hierarchy invariants, and thread-safety contracts:

```cpp
/// @class CircularByteRing
/// @brief Single-Producer Single-Consumer (SPSC) lock-free byte ring buffer.
/// @details Designed for low-latency zero-allocation transport byte streaming.
///          Employs cacheline separation (alignas(64)) between read and write indices
///          to prevent false sharing between reader and writer threads.
/// @note Thread-safety: Safe for exactly ONE producer thread calling write() and
///       exactly ONE consumer thread calling read() without external synchronization.
class CircularByteRing {
    // ...
};
```

---

## 3. Function & Method Template

Document purpose, parameters, return values, thread-safety, and specification references:

```cpp
/// @brief Encodes a Pelco-D command to set the camera optical zoom position.
/// @details Builds a 7-byte standard command frame targeting the calibrated zoom register.
///          Refer to Fujinon SX800 Pelco-D Specification Section 5.3.1 (Set Zoom Position).
/// @param[in] position Normalized raw encoder position in range [0x0000, 0x4000].
/// @return Valid 7-byte PelcoDFrame containing start byte, address, opcode, and checksum.
/// @note Guaranteed not to throw exceptions.
/// @see ProtocolParser::parseZoomPosition
/// @retval PelcoDFrame Successfully generated Pelco-D packet.
[[nodiscard]] PelcoDFrame buildSetZoomPosition(std::uint16_t position) noexcept;
```

---

## 4. Enum & Bitmask Template

Document each enumerator, underlying wire value, and specification reference:

```cpp
/// @enum OpticalFilter
/// @brief Day/Night optical cut filter selections.
/// @details Ref: Fujinon SX800 Pelco-D Specification Section 5.12.7 / 5.12.8.
enum class OpticalFilter : std::uint8_t {
    Visible = 0x01U, ///< Visible light band filter (Day mode default).
    IR      = 0x02U  ///< Infrared transmission filter (Night mode default).
};
```

---

## 5. Architectural Guidelines
1. **Core Library (`libs/FujinonSX800Core`):**
   - Document domain abstractions without mentioning Qt concepts or widgets.
   - Explicitly document concurrency guarantees (e.g. SPSC lock-free contracts, pacing intervals).
2. **Qt Adapter (`libs/FujinonSX800Qt`):**
   - Document signals and slots with their emission conditions and thread affinity (usually Qt GUI thread).
3. **Spec Traceability:** Whenever a method emits or parses a protocol byte sequence, include the exact specification section number (e.g., `@see Fujinon SX800 Spec Section 5.10.1`).

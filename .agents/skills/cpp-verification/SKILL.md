---
name: cpp-verification
description: >-
  Use this skill before outputting C++17 code snippets to verify safety, style, and compliance against the Fujinon SX800 Controller project checklist.
---

# C++17 Verification Checklist for Fujinon SX800 Controller

Before presenting any C++17 code snippet, internally verify it against this project-specific checklist:

## 1. Architecture & Layer Boundaries
1. **Three-Tier Isolation:**
   - `libs/FujinonSX800Core`: Pure C++17 library. **Zero Qt dependencies** (`#include <Q...>` is strictly banned). Must be usable in CLI, ROS, or headless environments.
   - `libs/FujinonSX800Qt`: Qt 6 adapter layer (`QFujinonCamera`, `QObject`, signals/slots).
   - `app-qt/`: Qt 6 GUI dashboard client. UI widgets must interact exclusively via `QFujinonCamera` or `FujinonCamera`, never accessing raw transport sockets/handles directly.
2. **Co-located File Organization:** Headers (`.h`) and implementations (`.cpp`) must reside in the same directory. Never introduce separate `include/` and `src/` directories.
3. **Header Guards:** Every header must use `#pragma once` at the very top. Never use `#ifndef / #define` include guards.
4. **Cross-Platform Portability:** All transport drivers (`SerialTransport`, `TcpTransport`) and core logic must compile and operate on both **Linux** (POSIX termios, BSD sockets) and **Windows** (Win32 Comm API, Winsock2).

## 2. Concurrency & I/O Mechanics
1. **Inbound Streaming:** Inbound byte stream reads must use the Single-Producer Single-Consumer (SPSC) lock-free circular byte ring (`CircularByteRing`) with cacheline padding (`alignas(64)`). Transport read threads push raw bytes lock-free and zero-allocation.
2. **Outbound Pacing:** Outbound camera commands must pass through the thread-safe command queue with ~15–20 ms inter-command delay per Pelco-D RS-485 specifications.

## 3. Resource Management & RAII
1. **Strict RAII:** All resources (memory, sockets, file descriptors, locks) must be tied to object lifetimes. Raw `new` and `delete` expressions are strictly **forbidden**.
2. **Smart Pointers:** Use `std::unique_ptr` by default for exclusive ownership. Use `std::make_unique<T>()` or `std::make_shared<T>()`. Never pass raw pointers to smart pointer constructors.
3. **Non-Owning Observers:** Raw pointers (`T*`) are strictly non-owning observers. Use `std::reference_wrapper` or const references where appropriate.

## 4. Types, Safety & Modern C++17
1. **Initialization:** All variables must be explicitly initialized upon declaration using `{}` to eliminate uninitialized state and prevent narrowing conversions.
2. **Fixed-Width Types:** Use explicit fixed-width integers from `<cstdint>` (`std::uint8_t`, `std::uint16_t`, `std::int32_t`, etc.) instead of basic types (`int`, `long`, `char`).
3. **Attributes:** Apply `[[nodiscard]]` to functions returning status, packet frames, or safety-critical data. Use `[[fallthrough]]` and `[[maybe_unused]]` explicitly where applicable.
4. **Error Handling:** No exceptions in critical I/O paths or protocol parsers. Report status via `bool`, `std::optional`, or error codes. Mark non-throwing functions as `noexcept`.
5. **No C-Style Casts:** Use `static_cast`, `const_cast`, or `reinterpret_cast`. Never use `(Type)value`.
6. **Function Names:** All function and method names must be concise and strictly **less than 30 characters**.
7. **Readability:** Keep indentation low. Avoid the Arrow Anti-Pattern using early returns and continues. Use enums instead of booleans for function parameters.

## 5. Build, Formatting & Compliance
1. **Compiler Diagnostics & Hardening:** Code must compile with zero warnings under `-Wall -Wextra -Wpedantic` and `/W4` on MSVC, passing all hardening and sanitizer checks in `cmake/CompilerFlags.cmake`.
2. **Clang-Format:** All formatting must conform to `.clang-format` (verified via `cmake --build build --target check-format`).
3. **Standards Compliance:** Comply with the intersection of AUTOSAR C++14/17, MISRA C++:2008, and SEI CERT C++ coding standards.
4. **Automated Verification:** All unit tests must pass 100% via `ctest --test-dir build --output-on-failure`.

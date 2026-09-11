---
name: git-commit
description: >-
  Use this skill when composing, reviewing, or generating Git commit messages conforming to the Fujinon SX800 Controller repository standards and Conventional Commits.
---

# Git Commit Instructions for Fujinon SX800 Controller

When composing commit messages for this repository, adhere strictly to the project's formatting rules, scoped component hierarchy, and Conventional Commits 1.0.0 principles.

---

## 1. Subject Line Format

- **Format:** `<Scope>: <verb in imperative> <summary>` (or `<type>(<scope>): <summary>`)
- **Scope Definition:** PascalCase / CamelCase denoting the specific subsystem or module:
  - `Core`: `libs/FujinonSX800Core` general architectural updates
  - `PelcoD`: Frame structure, packet packing/unpacking, checksums (`PelcoDFrame`)
  - `ProtocolBuilder`: Command frame creation and opcode emission (`ProtocolBuilder`)
  - `ProtocolParser`: Telemetry decoding and response parsing (`ProtocolParser`)
  - `CircularByteRing`: SPSC lock-free circular byte stream ring (`CircularByteRing`)
  - `Serial`: Serial/RS-485 transport driver (`SerialTransport`)
  - `Tcp`: TCP socket transport driver (`TcpTransport`)
  - `Camera`: High-level camera controller (`FujinonCamera`)
  - `MockCamera`: Virtual hardware camera simulation engine (`MockCameraDevice`)
  - `QtAdapter`: Qt 6 QObject adapter and signals/slots (`QFujinonCamera`)
  - `AppQt`: Qt 6 GUI dashboard client, tabs, and widgets (`app-qt/`)
  - `Build/CMake`: CMake configuration, modules, compiler flags, and hooks
  - `Tests`: Unit test suites and verification benchmarks (`tests/`)
  - `Docs`: Protocol documentation, Doxygen, and architectural guides
- **Multi-Component Scopes:** Use `/` for multi-component changes (e.g., `Core/Serial:`, `AppQt/OpticsTab:`, `Build/CMake:`).
- **Imperative Verb:** Immediately follow the colon with a lowercase imperative verb: `add`, `fix`, `implement`, `align`, `refactor`, `remove`, `configure`. (Test formula: "If applied, this commit will [your subject line]").
- **Punctuation:** Never end the subject line with a period (`.`).
- **Length:** Target <= 50 characters; 72 characters is the absolute hard limit.

---

## 2. Body Structure & Rules

- **Blank Line:** Separate the subject line from the body with exactly one blank line.
- **Manual Wrapping:** Manually hard-wrap all body lines at **72 characters**.
- **Content Focus:** Explain *what* and *why*, not the mechanics of the diff. Assume the reader can read the code; the commit message must document the context, constraints, and design rationale.
- **Component Breakdown:** Group technical details by component bullet points:
  ```text
  - libs/FujinonSX800Core/ComponentName:
    - Detail specific design shifts or API adjustments.
    - Explain non-obvious fixes or invariants maintained.
  - libs/FujinonSX800Qt:
    - Detail signal/slot additions or Qt property bindings.
  - app-qt/tabs/TabName:
    - Detail UI widget bindings or user interactions.
  - tests:
    - Describe new test cases and failure modes verified.
  ```

---

## 3. Breaking Changes & Footers

- Signal breaking API shifts with `!` in the header (e.g., `Core!:`) or a footer beginning with `BREAKING CHANGE:`.
- Include issue trackers or reviews where applicable (e.g., `Fixes: #42`, `Reviewed-by: Stavros Vagionitis`).

---

## 4. Repository Examples

### Protocol & Core Feature
```text
ProtocolBuilder: align image quality opcodes with SX800 spec v2.12.0

Correct query opcodes and command payloads for WDR, white balance,
fine settings, and day/night optical filters to match the official
Fujinon SX800 Pelco-D specification.

- libs/FujinonSX800Core/ProtocolBuilder.cpp:
  - Update buildSetWdr() opcode to 0x23 (Section 5.5.2).
  - Update buildSetWhiteBalance() opcode to 0x35 (Section 5.5.11).
  - Implement buildQueryFineSettings() using extended opcode 0xFD.
- tests/TestProtocolBuilder.cpp:
  - Add testCorrectedSpecOpcodes() covering all revised command bytes.
```

### Transport Bug Fix
```text
Tcp: fix socklen_t sign conversion on POSIX connect

Cast address length structure to socklen_t on POSIX platforms to resolve
compiler sign-conversion diagnostic under -Werror=sign-conversion while
preserving Win32 compatibility.

- libs/FujinonSX800Core/TcpTransport.cpp:
  - Use static_cast<socklen_t> for POSIX connect() and int for Windows.
```

### Build & Tooling Update
```text
Build/CMake: add compiler hardening and git pre-commit formatting hook

Integrate security hardening compiler options, sanitizers, automated
Git pre-commit formatting hooks, and clang-format CMake targets.

- cmake/CompilerFlags.cmake:
  - Add options for ENABLE_HARDENING, ENABLE_ASAN, ENABLE_UBSAN, and TSAN.
  - Apply stack protection, CET, PIE, Full RELRO, and non-exec stack.
- cmake/GitHooks.cmake:
  - Automate pre-commit hook installation into .git/hooks/pre-commit.
  - Define custom targets 'format' and 'check-format'.
```

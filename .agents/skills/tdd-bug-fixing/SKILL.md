---
name: tdd-bug-fixing
description: >-
  Use this skill when fixing bugs or regressions to enforce a strict Test-Driven Development (TDD) workflow in the Fujinon SX800 Controller.
---

# Test-Driven Bug Fixing for Fujinon SX800 Controller

Enforce strict Test-Driven Development (TDD) before implementing any bug fix or protocol correction.

---

## TDD Workflow

### 1. Reproduce with a Failing Test (RED)
- **Do not modify implementation code first.**
- Locate the relevant test target in `tests/`:
  - `TestPelcoDFrame.cpp`: Packet validation, payload bounds, modulo-256 checksums.
  - `TestProtocolBuilder.cpp`: Command opcode encoding and register values against the SX800 specification.
  - `TestProtocolParser.cpp`: Telemetry decoding, 7-byte/18-byte extended responses, status updates.
  - `TestOpticalTables.cpp`: Non-linear focal length, FOV, and IFOV interpolation.
  - `TestMockCamera.cpp`: End-to-end camera interrogation, pacing, and simulated device behavior.
  - `TestCircularByteRing.cpp`: SPSC multi-threaded streaming, wrap-around, and boundary conditions.
- Write a minimal, targeted assertion that reproduces the reported bug or protocol discrepancy.
- If testing high-level camera behavior without physical serial/network hardware, leverage `MockCameraDevice`.

### 2. Observe the Failure
- Build and execute the test suite:
  ```bash
  cmake --build build && ctest --test-dir build --output-on-failure
  ```
- Confirm that the test fails **specifically** for the expected reason (e.g. incorrect checksum, mismatched opcode, unparsed response field).

### 3. Implement the Minimal Fix (GREEN)
- Write the minimal code fix required to resolve the bug according to project standards:
  - Adhere to the Three-Tier Architecture (zero Qt in `FujinonSX800Core`).
  - Keep functions under 30 characters and avoid raw pointers for ownership.
  - Avoid exceptions in critical paths (use `bool`, `std::optional`, or error codes).
- Re-run the tests:
  ```bash
  ctest --test-dir build --output-on-failure
  ```
- Verify that the target test passes and that **100%** of the CTest suite passes without regressions.

### 4. Verify Hardening & Formatting (REFACTOR)
- Check that code compiles with zero warnings under `-Wall -Wextra -Wpedantic` and `/W4`.
- Validate formatting compliance:
  ```bash
  cmake --build build --target check-format
  ```
- If concurrency code was touched, run with ThreadSanitizer or stress tests.

### 5. Document Root Cause & Implications
- Provide a clear, technical explanation:
  1. **Root Cause:** What underlying condition or specification misalignment caused the defect?
  2. **Resolution:** How the change corrects the behavior while preserving architectural invariants.
  3. **Implications:** Side-effects on telemetry, downstream GUI components, or transport timing.

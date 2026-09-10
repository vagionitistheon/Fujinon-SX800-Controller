# Git Commit Instructions

When writing a commit message, follow these strict repository rules:

## 1. Subject Line Format
- **Format:** `<Scope>: <verb in imperative> <summary>`
  - Scope is PascalCase/CamelCase identifying the affected component:
    - `Core`: `libs/FujinonSX800Core` general updates
    - `PelcoD`: Frame structure, packing, unpacking, checksums
    - `ProtocolBuilder`: Command frame creation
    - `ProtocolParser`: Response parsing and telemetry decoding
    - `CircularByteRing`: SPSC lock-free circular byte stream ring
    - `Serial`: Serial/RS-485 transport driver
    - `Tcp`: TCP socket transport driver
    - `Camera`: High-level `FujinonCamera` controller
    - `MockCamera`: Virtual camera simulation engine
    - `QtAdapter`: `libs/FujinonSX800Qt` QObject wrapper
    - `AppQt`: Qt 6 GUI application and UI widgets/tabs
    - `Tests`: Unit tests and verification suites
    - `Docs`: Protocol documentation or architectural specs
  - Use `/` for multi-component changes (e.g., `Core/Serial:`, `AppQt/OpticsTab:`).
  - Use brace expansion for sibling units (e.g., `SerialTransport{,.cpp}:`).
- **Verb:** Use lowercase imperative mood immediately after the colon (`fix`, `add`, `implement`, `refactor`, `align`, `remove`, `configure`).
- **Punctuation:** Do not end the subject line with a period.
- **Length:** Target <= 50 characters when possible; 72 characters is the absolute hard limit (especially with multi-component scopes).

## 2. Body Structure
- **Blank Line:** Separate subject and body with exactly one blank line.
- **Wrapping:** Manually hard-wrap all body lines at 72 characters.
- **Summary Paragraph:** Start with a brief overview explaining the problem, context, and rationale (*what* and *why*, not raw code diffs).
- **Component Breakdown:** Group technical details by component bullet points:
  ```text
  - libs/FujinonSX800Core/ComponentName:
    - Detail specific design shifts or API adjustments.
    - Explain non-obvious fixes or invariants maintained.
  ```


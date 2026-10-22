# AGENTS.md

## Project

- Qt 6 / C++ project.
- Follow existing project patterns where they conflict with this file.
- Keep changes minimal and focused.
- Do not edit generated build artifacts.

## Naming

- Functions, methods, signals, and slots: `snake_case`
- Local variables and parameters: `camelCase`
- Private member variables: `m_camelCase`
- Types, classes, structs, and enums: `PascalCase`
- Constants: `kPascalCase`
- Files: `snake_case.cpp` and `snake_case.h`

## Qt

- Prefer Qt 6 APIs.
- Use typed signal/slot connections.
- Give heap-allocated `QObject` instances an appropriate parent where practical.
- Do not block the GUI thread with long-running work.
- Keep UI code separate from business logic where practical.

## Validation

- Build affected targets and run relevant tests when available.
- State any validation that could not be run.
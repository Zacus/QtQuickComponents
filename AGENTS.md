# Repository Guidelines

## Project Structure & Module Organization

This repository is a reusable Qt Quick component library. C++ backing types live in `src/`, including theme support, timeline models, ruler models, and viewport logic. QML controls live in `qml/`, with files such as `Button.qml`, `TextField.qml`, `TimelineView.qml`, and related timeline subcomponents. CMake package support is under `cmake/`. Generated files and local build output belong in `build/` and should not be edited directly.

## Build, Test, and Development Commands

Configure the project from the repository root:

```sh
cmake -S . -B build
```

Build the library and QML module:

```sh
cmake --build build
```

Install into the configured CMake prefix when needed:

```sh
cmake --install build
```

The library requires Qt 6.4+, CMake 3.21+, and C++17. If a consuming app needs to load the built QML module, ensure its `QML_IMPORT_PATH` includes the build directory or installed module path.

## Coding Style & Naming Conventions

Use C++17 and Qt idioms. Keep public C++ types in `PascalCase` files matching the class name, for example `TimelineModel.h` and `TimelineModel.cpp`. Use Qt naming patterns for properties, signals, and slots. QML component filenames are `PascalCase.qml`; internal properties and handlers use normal QML `camelCase`. Preserve the existing 4-space indentation style in QML and CMake files, and keep comments concise and useful.

## Testing Guidelines

There is currently no dedicated automated test suite in the repository. Before submitting changes, at minimum run a clean CMake configure and build. For QML changes, validate the affected component in a consuming Qt Quick app and check both light and dark theme behavior when relevant. If tests are added later, place them under a top-level `tests/` directory and wire them into CTest so contributors can run `ctest --test-dir build`.

## Commit & Pull Request Guidelines

Recent commits use short Chinese category prefixes such as `[功能修改]`, `[功能优化]`, and `[bug修复]`. Follow that style when appropriate, then describe the user-visible change or bug fixed. Keep commits focused and avoid mixing generated build artifacts with source edits.

Pull requests should include a concise summary, affected components, validation steps, and screenshots or short recordings for visible QML behavior changes. Link related issues when available and call out any API, import URI, or install-layout changes.

## Agent-Specific Instructions

Do not modify files in `build/` unless explicitly asked. Prefer updating source lists in `CMakeLists.txt` whenever adding C++ or QML files so the module remains consumable through `QuickUI.Components 1.0`.

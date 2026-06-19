# SingleWnd Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first production-ready slice of `SingleWnd`: public view model contract, QML layer composition, status/OSD/border/title UI, click event forwarding, and a placeholder `VideoSurface` API ready for the later RHI renderer.

**Architecture:** `WndViewModel` is a public `QuickUI.Components` C++ QML type because consuming apps need to instantiate and drive it. `SingleWnd.qml` is the public component; it imports `QuickUI.Components.impl` for private layer components and `VideoSurface`. The real RHI/YUV renderer is isolated as a later phase so the UI/API contract can compile and be tested independently first.

**Tech Stack:** Qt 6.4+, C++17, QML, CMake `qt_add_qml_module`, Qt Test, Qt QuickTest.

---

## File Structure

- Create `include/QtQuickComponents/WndViewModel.h`: public QML-facing state object, enums, properties, and event signals.
- Create `src/monitor/WndViewModel.cpp`: value setters that emit notify signals only when values change.
- Create `src/monitor/VideoSurface.h`: internal `QQuickItem` placeholder API with `channelId` and `videoSink`; owned by QML parent.
- Create `src/monitor/VideoSurface.cpp`: item setup and property notification only. No render node yet.
- Create `src/monitor/SingleWnd.qml`: public layered component that binds to `vm` and re-emits view-model signals through handlers.
- Create `src/monitor/NoSignalLayer.qml`: internal signal-state overlay.
- Create `src/monitor/OSDLayer.qml`: internal four-corner OSD renderer.
- Create `src/monitor/BorderLayer.qml`: internal active/alarm border.
- Create `src/monitor/TitleBar.qml`: internal hover toolbar.
- Modify `CMakeLists.txt`: add source lists, QML aliases, include dirs, and module file registrations.
- Modify `tests/CMakeLists.txt`: add a C++ `WndViewModel` test.
- Create `tests/tst_wnd_view_model.cpp`: verify property changes and notify signals.
- Create `tests/qml/tst_single_wnd.qml`: verify QML construction, visibility state, OSD, and event signal forwarding.
- Modify `tests/verify_qml_api_surface.cmake`: include the new public/internal QML files and internal C++ helper.

## Task 1: WndViewModel Contract

**Files:**
- Create: `include/QtQuickComponents/WndViewModel.h`
- Create: `src/monitor/WndViewModel.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/tst_wnd_view_model.cpp`

- [ ] **Step 1: Write the failing C++ test**

```cpp
#include <QtTest/QtTest>
#include "WndViewModel.h"

class tst_WndViewModel : public QObject
{
    Q_OBJECT

private slots:
    void defaultsMatchEmptyWindow();
    void propertySettersEmitOnlyOnChange();
};

void tst_WndViewModel::defaultsMatchEmptyWindow()
{
    WndViewModel vm;
    QCOMPARE(vm.wndId(), 0);
    QCOMPARE(vm.channelId(), -1);
    QCOMPARE(vm.channelName(), QString());
    QCOMPARE(vm.isActive(), false);
    QCOMPARE(vm.isRecording(), false);
    QCOMPARE(vm.alarmLevel(), WndViewModel::None);
    QCOMPARE(vm.signalState(), WndViewModel::NoSignal);
    QCOMPARE(vm.osdModel(), QVariantList());
    QCOMPARE(vm.videoSink(), nullptr);
}

void tst_WndViewModel::propertySettersEmitOnlyOnChange()
{
    WndViewModel vm;
    QSignalSpy channelSpy(&vm, &WndViewModel::channelIdChanged);
    QSignalSpy signalSpy(&vm, &WndViewModel::signalStateChanged);

    vm.setChannelId(7);
    vm.setChannelId(7);
    vm.setSignalState(WndViewModel::Normal);

    QCOMPARE(vm.channelId(), 7);
    QCOMPARE(channelSpy.count(), 1);
    QCOMPARE(vm.signalState(), WndViewModel::Normal);
    QCOMPARE(signalSpy.count(), 1);
}

QTEST_MAIN(tst_WndViewModel)
#include "tst_wnd_view_model.moc"
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target tst_wnd_view_model`

Expected: FAIL because `WndViewModel.h` and target registration do not exist.

- [ ] **Step 3: Implement the minimal view model**

Add `WndViewModel` with `QML_ELEMENT`, the enums from `SingleWnd_designer.md`, read/write properties for tests and QML fixtures, and event signals. Ownership model: `videoSink` is a non-owning `QPointer<QObject>` observer; the sink lifetime is owned by the provider that assigns it.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build --target tst_wnd_view_model && ./build/tests/tst_wnd_view_model`

Expected: PASS with no warnings.

## Task 2: VideoSurface Placeholder API

**Files:**
- Create: `src/monitor/VideoSurface.h`
- Create: `src/monitor/VideoSurface.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/verify_qml_api_surface.cmake`

- [ ] **Step 1: Write a QML API expectation**

Add `VideoSurface` to `_qtc_impl_cpp_types` in `tests/verify_qml_api_surface.cmake`.

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build -R QmlApiSurface --output-on-failure`

Expected: FAIL because `QuickUI.Components.impl/VideoSurface` is not exported.

- [ ] **Step 3: Implement the placeholder item**

Add an internal `QQuickItem` type with `channelId` and `videoSink` properties. Set `setFlag(ItemHasContents, false)` for phase 1. Do not implement `QSGRenderNode` in this task.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build && ctest --test-dir build -R QmlApiSurface --output-on-failure`

Expected: PASS.

## Task 3: QML Layer Components

**Files:**
- Create: `src/monitor/NoSignalLayer.qml`
- Create: `src/monitor/OSDLayer.qml`
- Create: `src/monitor/BorderLayer.qml`
- Create: `src/monitor/TitleBar.qml`
- Modify: `CMakeLists.txt`
- Modify: `tests/verify_qml_api_surface.cmake`

- [ ] **Step 1: Write the failing QML construction test**

Create `tests/qml/tst_single_wnd.qml` with a `SingleWnd` fixture and checks for `signalState`, `osdModel`, and title buttons. Initially this fails because `SingleWnd` and layers do not exist.

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target tst_qml_components && ctest --test-dir build -R QmlComponents --output-on-failure`

Expected: FAIL with unknown `SingleWnd`.

- [ ] **Step 3: Implement internal layers**

`NoSignalLayer` exposes `signalState`, text/icon override properties, and only shows NoSignal/Connecting content. `OSDLayer` groups `osdModel` entries by `position`. `BorderLayer` exposes `isActive` and `alarmLevel`, with Critical > Warning > Active precedence. `TitleBar` exposes `channelName`, `isRecording`, and emits `screenshotClicked`, `recordClicked`, and `closeClicked`.

- [ ] **Step 4: Re-run QML test**

Run: `ctest --test-dir build -R QmlComponents --output-on-failure`

Expected: still FAIL until `SingleWnd.qml` is added in Task 4.

## Task 4: SingleWnd Public Component

**Files:**
- Create: `src/monitor/SingleWnd.qml`
- Modify: `CMakeLists.txt`
- Modify: `tests/verify_qml_api_surface.cmake`
- Modify: `tests/qml/tst_single_wnd.qml`

- [ ] **Step 1: Complete the failing QML test**

Test these behaviors: construction with `WndViewModel`, `VideoSurface` visible only when `signalState === WndViewModel.Normal`, no-signal layer visible in `NoSignal`, OSD text appears, title buttons forward to `WndViewModel` signals, and single click emits `clicked`.

- [ ] **Step 2: Run test to verify it fails for missing implementation**

Run: `ctest --test-dir build -R QmlComponents --output-on-failure`

Expected: FAIL on missing bindings or missing event forwarding.

- [ ] **Step 3: Implement `SingleWnd.qml`**

Use an `Item` root with `property WndViewModel vm: null`, six z-ordered children, and handlers that only emit `vm` signals. Object names should be stable for tests: `videoSurface`, `noSignalLayer`, `osdLayer`, `borderLayer`, `titleBar`, and `dragLayer`.

- [ ] **Step 4: Run QML and API tests**

Run: `cmake --build build && ctest --test-dir build -R "QmlComponents|QmlApiSurface" --output-on-failure`

Expected: PASS.

## Task 5: Full Verification

**Files:**
- No new files.

- [ ] **Step 1: Run the full configured test suite**

Run: `ctest --test-dir build --output-on-failure`

Expected: PASS.

- [ ] **Step 2: Inspect changed files**

Run: `git status --short`

Expected: only SingleWnd implementation files, CMake/test updates, and the plan are new or modified, plus pre-existing user changes remain untouched.

## Deferred Phase: Real RHI/YUV Renderer

The design document's `QSGRenderNode` + global RHI renderer is intentionally deferred until the API and QML contract are stable. That phase should get a separate plan with these tasks:

- `GlobalVideoRenderer` lifecycle, thread ownership, frame queue, and teardown tests.
- `VideoSurface::updatePaintNode`/`QSGRenderNode` integration.
- QRhi texture upload path and shader pipeline.
- Pure color frame test, resize/aspect test, channel switch test, and multi-channel performance measurement.

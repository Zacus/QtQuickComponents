# SingleWnd Frame Source Selection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Teach `VideoSurface` to identify whether the current frame should be rendered from RGBA texture data or a preserved YUV420 snapshot, preparing the split between fallback texture rendering and future QRhi shader rendering.

**Architecture:** `VideoSurface` remains the QML-facing placeholder for video output. It observes `GlobalVideoRenderer::frameReady`, asks the renderer whether the current channel has a YUV420 snapshot, and records an internal `activeFrameFormat`. The existing `QSGSimpleTextureNode` RGBA fallback remains the actual drawing path until a dedicated YUV render node is added.

**Tech Stack:** Qt 6.4+, C++17, Qt Quick SceneGraph, Qt Test.

---

## File Structure

- Modify `src/monitor/VideoSurface.h`: add `FrameFormat`, `activeFrameFormat`, and updated frame state helpers.
- Modify `src/monitor/VideoSurface.cpp`: choose frame format from `GlobalVideoRenderer` state.
- Modify `tests/tst_video_surface.cpp`: verify RGBA/YUV source selection and replacement semantics.
- Modify `tests/qml/tst_single_wnd.qml`: verify a fresh surface reports `NoFrame`.

## Task 1: Active Frame Source State

**Files:**
- Modify: `tests/tst_video_surface.cpp`
- Modify: `tests/qml/tst_single_wnd.qml`
- Modify: `src/monitor/VideoSurface.h`
- Modify: `src/monitor/VideoSurface.cpp`

- [ ] **Step 1: Write failing tests**

Add expectations for `VideoSurface::activeFrameFormat()` after RGBA push, YUV420 push, RGBA replacement, and clear.

- [ ] **Step 2: Run red verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_video_surface`

Expected: FAIL because `activeFrameFormat()` and `FrameFormat` do not exist.

- [ ] **Step 3: Implement source selection**

Add enum values `NoFrame`, `RgbaFrame`, and `Yuv420Frame`. `refreshFrameState()` and `frameReady` should set `Yuv420Frame` when `renderer.hasYuv420Frame(channelId)` is true, otherwise `RgbaFrame` when a frame serial exists.

- [ ] **Step 4: Run green verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_video_surface tst_qml_components && ctest --test-dir /private/tmp/qtc-singlewnd-build -R "VideoSurface|QmlComponents" --output-on-failure`

Expected: PASS.

## Deferred Work

The next phase can introduce a render node implementation that consumes `Yuv420Snapshot` when `activeFrameFormat == Yuv420Frame`.

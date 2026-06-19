# SingleWnd YUV Snapshot Phase Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve validated YUV420 plane snapshots in `GlobalVideoRenderer` so the later QRhi shader renderer can consume Y, U, and V planes directly instead of relying only on CPU-converted RGBA frames.

**Architecture:** `GlobalVideoRenderer` keeps two per-channel stores guarded by the same mutex: RGBA `FrameSnapshot` for the current `QSGSimpleTextureNode` path and `Yuv420Snapshot` for the future GPU path. `pushYuv420Frame()` validates and deep-copies YUV planes, assigns one serial, stores both YUV and RGBA snapshots with that serial, then emits `frameReady`. `pushFrame()` stores only RGBA and clears any stale YUV snapshot for that channel.

**Tech Stack:** Qt 6.4+, C++17, QByteArray, QImage, QMutex, Qt Test.

---

## File Structure

- Modify `src/monitor/GlobalVideoRenderer.h`: add `Yuv420Snapshot`, `yuv420Snapshot()`, and `hasYuv420Frame()`.
- Modify `src/monitor/GlobalVideoRenderer.cpp`: store copied YUV plane snapshots and clear them consistently.
- Modify `tests/tst_global_video_renderer.cpp`: verify YUV snapshot copy/lifetime and clear semantics.
- Modify `tests/tst_video_surface.cpp`: verify `VideoSurface` observes YUV frame serials through existing `frameReady`.

## Task 1: YUV Snapshot Store

**Files:**
- Modify: `tests/tst_global_video_renderer.cpp`
- Modify: `src/monitor/GlobalVideoRenderer.h`
- Modify: `src/monitor/GlobalVideoRenderer.cpp`

- [ ] **Step 1: Write failing tests**

Add tests that verify `pushYuv420Frame()` stores a deep-copied `Yuv420Snapshot`, `clearChannel()` removes it, and `pushFrame()` replaces it with an RGBA-only frame for the same channel.

- [ ] **Step 2: Run red verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_global_video_renderer`

Expected: FAIL because `Yuv420Snapshot`, `yuv420Snapshot()`, and `hasYuv420Frame()` do not exist.

- [ ] **Step 3: Implement snapshot storage**

Add a second `QHash<int, Yuv420Snapshot>`. Copy plane data before storing. Use the same serial for YUV and RGBA snapshots produced by one `pushYuv420Frame()` call. `pushFrame()` removes `m_yuvFrames[channelId]`.

- [ ] **Step 4: Run green verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_global_video_renderer && /private/tmp/qtc-singlewnd-build/tests/tst_global_video_renderer`

Expected: PASS.

## Task 2: VideoSurface Observation Coverage

**Files:**
- Modify: `tests/tst_video_surface.cpp`

- [ ] **Step 1: Write/extend the test**

Extend `VideoSurfaceTest::followsMatchingRendererFrames()` so a matching `pushYuv420Frame()` updates `hasFrame` and advances `currentSerial`.

- [ ] **Step 2: Run targeted verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_video_surface && /private/tmp/qtc-singlewnd-build/tests/tst_video_surface`

Expected: PASS after Task 1 because `pushYuv420Frame()` still emits `frameReady`.

## Deferred Work

The next phase can introduce a render abstraction that chooses RGBA texture node or YUV render node. Once that is in place, the YUV snapshot will be consumed by a `QSGRenderNode`/QRhi shader path.

# SingleWnd YUV Node Integration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Connect `Yuv420RenderNode` to `VideoSurface::updatePaintNode()` when the active frame source is YUV420.

**Architecture:** `VideoSurface` continues to observe `GlobalVideoRenderer` and choose an active frame format. When a YUV420 snapshot is current, `updatePaintNode()` creates a SceneGraph subtree containing `Yuv420RenderNode`; while the shader render path is still a no-op, the existing CPU-converted RGBA snapshot remains available as fallback for later composition.

**Tech Stack:** Qt 6.4+, C++17, Qt Quick SceneGraph, Qt Test.

---

## File Structure

- Modify `src/monitor/VideoSurface.cpp`: create YUV render node trees for YUV snapshots.
- Modify `tests/tst_video_surface.cpp`: verify `updatePaintNode()` creates a `Yuv420RenderNode` with correct rect and serial.

## Task 1: YUV Node Selection

**Files:**
- Modify: `tests/tst_video_surface.cpp`
- Modify: `src/monitor/VideoSurface.cpp`

- [ ] **Step 1: Write failing test**

Extend `VideoSurfaceTest` so a YUV frame followed by `updatePaintNode(nullptr, nullptr)` returns a node containing `Yuv420RenderNode` with `rect() == surface.boundingRect()` and the current serial.

- [ ] **Step 2: Run red verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_video_surface`

Expected: FAIL because `VideoSurface::updatePaintNode()` still returns `nullptr` without a window and never creates `Yuv420RenderNode`.

- [ ] **Step 3: Implement YUV branch**

When `activeFrameFormat == Yuv420Frame`, fetch `renderer.yuv420Snapshot(channelId)`, delete any old node, and return a new `Yuv420RenderNode` or container subtree. Keep the RGBA texture path unchanged for `RgbaFrame`.

- [ ] **Step 4: Run green verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_video_surface && /private/tmp/qtc-singlewnd-build/tests/tst_video_surface`

Expected: PASS.

## Deferred Work

The next phase should implement QRhi resource allocation and shader commands inside `Yuv420RenderNode::render()`.

# SingleWnd YUV Render Node Skeleton Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a `QSGRenderNode`-based YUV render node skeleton that owns a YUV420 snapshot and exposes render bounds, preparing the next phase where QRhi shader commands are emitted.

**Architecture:** `Yuv420RenderNode` is an internal SceneGraph node, not a QML type. It stores a deep-copied `GlobalVideoRenderer::Yuv420Snapshot`, a target rect, and reports `BoundedRectRendering`. Its `render()` is intentionally a no-op in this phase so `VideoSurface` can keep using the RGBA fallback until shader resources are implemented.

**Tech Stack:** Qt 6.4+, C++17, `QSGRenderNode`, Qt Test.

---

## File Structure

- Create `src/monitor/Yuv420RenderNode.h`: internal `QSGRenderNode` subclass.
- Create `src/monitor/Yuv420RenderNode.cpp`: snapshot/rect storage and no-op render implementation.
- Modify `CMakeLists.txt`: add the node to `QtQuickComponentsImpl`.
- Modify `tests/CMakeLists.txt`: add `tst_yuv420_render_node`.
- Create `tests/tst_yuv420_render_node.cpp`: verify node metadata and snapshot copying.
- Modify `tests/verify_public_headers.cmake`: keep the new internal header out of public headers.

## Task 1: Render Node Skeleton

**Files:**
- Create: `src/monitor/Yuv420RenderNode.h`
- Create: `src/monitor/Yuv420RenderNode.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/tst_yuv420_render_node.cpp`
- Modify: `tests/verify_public_headers.cmake`

- [ ] **Step 1: Write failing test**

Test construction, `rect()`, `flags()`, serial propagation, snapshot copy, and no-op `render(nullptr)`.

- [ ] **Step 2: Run red verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_yuv420_render_node`

Expected: FAIL because `Yuv420RenderNode.h` does not exist.

- [ ] **Step 3: Implement skeleton**

Add a non-QObject `Yuv420RenderNode : public QSGRenderNode`. Store copied snapshot data in the constructor/update method, return the target rect from `rect()`, return `BoundedRectRendering` from `flags()`, and leave `render()` empty.

- [ ] **Step 4: Run green verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_yuv420_render_node && /private/tmp/qtc-singlewnd-build/tests/tst_yuv420_render_node`

Expected: PASS.

## Deferred Work

The next phase should allocate QRhi resources for Y, U, and V plane textures and implement shader-backed `render()`.

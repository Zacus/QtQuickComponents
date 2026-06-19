# SingleWnd YUV Input Phase Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a validated YUV420 frame input path to `GlobalVideoRenderer` so decoder-style planar frames can feed the existing `VideoSurface` texture display path.

**Architecture:** `GlobalVideoRenderer` gains an internal `Yuv420Frame` value type that owns copied Y, U, and V planes plus stride/size metadata. `pushYuv420Frame()` validates dimensions and plane buffers, converts using BT.601 limited-range math into RGBA, and stores the result through the same snapshot path used by `pushFrame()`. The current `VideoSurface` SceneGraph texture path remains unchanged.

**Tech Stack:** Qt 6.4+, C++17, QImage, QByteArray, Qt Test.

---

## File Structure

- Modify `src/monitor/GlobalVideoRenderer.h`: add `Yuv420Frame` and `pushYuv420Frame()`.
- Modify `src/monitor/GlobalVideoRenderer.cpp`: add validation and YUV420-to-RGBA conversion.
- Modify `tests/tst_global_video_renderer.cpp`: add conversion and invalid-input coverage.

## Task 1: YUV420 Input Contract

**Files:**
- Modify: `tests/tst_global_video_renderer.cpp`
- Modify: `src/monitor/GlobalVideoRenderer.h`
- Modify: `src/monitor/GlobalVideoRenderer.cpp`

- [ ] **Step 1: Write failing tests**

Add tests that push a 2x2 YUV420 white frame and verify the stored RGBA pixels are white, then push malformed YUV frames and verify they are rejected without creating a snapshot.

- [ ] **Step 2: Run red verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_global_video_renderer`

Expected: FAIL because `GlobalVideoRenderer::Yuv420Frame` and `pushYuv420Frame()` do not exist.

- [ ] **Step 3: Implement minimal conversion**

Use BT.601 limited range:

```cpp
C = Y - 16
D = U - 128
E = V - 128
R = clamp((298 * C + 409 * E + 128) >> 8)
G = clamp((298 * C - 100 * D - 208 * E + 128) >> 8)
B = clamp((298 * C + 516 * D + 128) >> 8)
```

Reject negative channel IDs, odd/non-positive dimensions, non-positive strides, and buffers smaller than the required plane footprint.

- [ ] **Step 4: Run green verification**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_global_video_renderer && /private/tmp/qtc-singlewnd-build/tests/tst_global_video_renderer`

Expected: PASS.

## Deferred Work

This still uses CPU conversion into RGBA. The next renderer phase should move the conversion to the render path by storing YUV plane snapshots and drawing them with a `QSGRenderNode`/QRhi shader pipeline.

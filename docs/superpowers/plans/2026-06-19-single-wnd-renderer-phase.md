# SingleWnd Renderer Phase Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add the first real video-rendering backend for `SingleWnd`: a thread-safe global frame store and a `VideoSurface` SceneGraph node that displays the latest frame for its `channelId`.

**Architecture:** `GlobalVideoRenderer` is an internal QObject service with per-channel frame snapshots guarded by a mutex. Decoder or test threads can call `pushFrame()` from any thread; the renderer emits `frameReady(channelId, serial)` after storing a copy. `VideoSurface` observes a renderer through its existing `videoSink` property, updates only for matching channel IDs, and creates a `QSGTexture` from the latest frame on the render path.

**Tech Stack:** Qt 6.4+, C++17, Qt Quick SceneGraph, QImage, QMutex, Qt Test.

---

## File Structure

- Create `src/monitor/GlobalVideoRenderer.h`: internal renderer service, `FrameSnapshot` value type, thread-safe frame API.
- Create `src/monitor/GlobalVideoRenderer.cpp`: mutex-protected frame storage and notification.
- Modify `src/monitor/VideoSurface.h`: add `updatePaintNode`, renderer connection helpers, and cached frame serial.
- Modify `src/monitor/VideoSurface.cpp`: connect to `GlobalVideoRenderer`, request updates on matching frames, and create `QSGSimpleTextureNode`.
- Modify `CMakeLists.txt`: add renderer source/header to `QtQuickComponentsImpl`.
- Modify `tests/CMakeLists.txt`: add `tst_global_video_renderer`.
- Create `tests/tst_global_video_renderer.cpp`: verify frame copy, channel isolation, clear behavior, and worker-thread push.

## Task 1: GlobalVideoRenderer Frame Store

**Files:**
- Create: `src/monitor/GlobalVideoRenderer.h`
- Create: `src/monitor/GlobalVideoRenderer.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/tst_global_video_renderer.cpp`

- [ ] **Step 1: Write the failing C++ test**

```cpp
#include <QtTest/QtTest>
#include <thread>

#include "GlobalVideoRenderer.h"

class GlobalVideoRendererTest : public QObject
{
    Q_OBJECT

private slots:
    void storesIndependentChannelFrames();
    void clearChannelRemovesOnlyThatChannel();
    void pushFrameCanRunFromWorkerThread();
};

void GlobalVideoRendererTest::storesIndependentChannelFrames()
{
    GlobalVideoRenderer renderer;
    QImage red(2, 2, QImage::Format_RGBA8888);
    red.fill(Qt::red);
    QImage blue(2, 2, QImage::Format_RGBA8888);
    blue.fill(Qt::blue);

    QVERIFY(renderer.pushFrame(1, red));
    QVERIFY(renderer.pushFrame(2, blue));

    const auto one = renderer.frameSnapshot(1);
    const auto two = renderer.frameSnapshot(2);
    QVERIFY(one.isValid());
    QVERIFY(two.isValid());
    QCOMPARE(one.image.pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(two.image.pixelColor(0, 0), QColor(Qt::blue));
    QVERIFY(two.serial > one.serial);
}

void GlobalVideoRendererTest::clearChannelRemovesOnlyThatChannel()
{
    GlobalVideoRenderer renderer;
    QImage image(1, 1, QImage::Format_RGBA8888);
    image.fill(Qt::green);

    renderer.pushFrame(1, image);
    renderer.pushFrame(2, image);
    renderer.clearChannel(1);

    QVERIFY(!renderer.frameSnapshot(1).isValid());
    QVERIFY(renderer.frameSnapshot(2).isValid());
}

void GlobalVideoRendererTest::pushFrameCanRunFromWorkerThread()
{
    GlobalVideoRenderer renderer;
    QImage image(4, 4, QImage::Format_RGBA8888);
    image.fill(Qt::yellow);

    std::thread worker([&renderer, image]() {
        QVERIFY(renderer.pushFrame(9, image));
    });
    worker.join();

    const auto snapshot = renderer.frameSnapshot(9);
    QVERIFY(snapshot.isValid());
    QCOMPARE(snapshot.image.size(), QSize(4, 4));
    QCOMPARE(snapshot.image.pixelColor(0, 0), QColor(Qt::yellow));
}

QTEST_APPLESS_MAIN(GlobalVideoRendererTest)
#include "tst_global_video_renderer.moc"
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_global_video_renderer`

Expected: FAIL because `GlobalVideoRenderer.h` and the test target do not exist yet.

- [ ] **Step 3: Implement the frame store**

Add `GlobalVideoRenderer` with `pushFrame(int, const QImage&)`, `frameSnapshot(int) const`, `clearChannel(int)`, and `clear()`. Reject negative channel IDs and null images. Convert accepted images to `QImage::Format_RGBA8888`, store a deep copy under `QMutex`, increment a monotonic serial, then emit `frameReady(channelId, serial)` after releasing the lock.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build /private/tmp/qtc-singlewnd-build --target tst_global_video_renderer && /private/tmp/qtc-singlewnd-build/tests/tst_global_video_renderer`

Expected: PASS.

## Task 2: VideoSurface SceneGraph Integration

**Files:**
- Modify: `src/monitor/VideoSurface.h`
- Modify: `src/monitor/VideoSurface.cpp`
- Modify: `tests/qml/tst_single_wnd.qml`

- [ ] **Step 1: Add observable rendering state to the QML test**

Extend `tst_single_wnd.qml` to verify `videoSurface.hasFrame` is false before any frame arrives and remains bound to `channelId`.

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir /private/tmp/qtc-singlewnd-build -R QmlComponents --output-on-failure`

Expected: FAIL because `VideoSurface.hasFrame` does not exist.

- [ ] **Step 3: Implement VideoSurface frame observation**

Add read-only `hasFrame` and `currentSerial` properties. When `videoSink` is a `GlobalVideoRenderer`, connect `frameReady` to a slot/lambda on `VideoSurface`; if `channelId` matches, update `hasFrame`, cache the serial, and call `update()`. On `channelId` change, refresh from the renderer.

- [ ] **Step 4: Implement `updatePaintNode`**

Use `QSGSimpleTextureNode`. On each paint update, fetch a snapshot for the current channel. If no frame exists, delete the old node and return `nullptr`. If a frame exists, create a texture using `window()->createTextureFromImage(snapshot.image)`, assign it to the node with ownership, and set the rect to `boundingRect()`.

- [ ] **Step 5: Run tests**

Run: `cmake --build /private/tmp/qtc-singlewnd-build && ctest --test-dir /private/tmp/qtc-singlewnd-build -R "GlobalVideoRenderer|QmlComponents|QmlApiSurface" --output-on-failure`

Expected: PASS.

## Deferred Work

This phase does not yet implement the final YUV three-plane QRhi shader pipeline. That should follow once the frame store and `VideoSurface` channel lifecycle are stable:

- Add YUV frame value types with plane strides and color range metadata.
- Add QRhi texture upload tests around resource lifecycle.
- Replace `QSGSimpleTextureNode` with a `QSGRenderNode` that draws through a shared YUV-to-RGB pipeline.
- Add visual/offscreen tests for aspect fit/fill and channel switching.

#include <QtTest/QtTest>

#include "GlobalVideoRenderer.h"
#include "VideoSurface.h"
#include "Yuv420RenderNode.h"

class VideoSurfaceTest : public QObject
{
    Q_OBJECT

private slots:
    void followsMatchingRendererFrames();
    void createsYuvRenderNodeForYuvFrames();
};

void VideoSurfaceTest::followsMatchingRendererFrames()
{
    GlobalVideoRenderer renderer;
    VideoSurface surface;
    surface.setChannelId(5);
    surface.setVideoSink(&renderer);

    QSignalSpy frameSpy(&surface, &VideoSurface::frameStateChanged);
    QCOMPARE(surface.activeFrameFormat(), VideoSurface::NoFrame);
    QImage image(3, 3, QImage::Format_RGBA8888);
    image.fill(Qt::magenta);

    QVERIFY(renderer.pushFrame(4, image));
    QCOMPARE(surface.hasFrame(), false);
    QCOMPARE(surface.currentSerial(), quint64(0));

    QVERIFY(renderer.pushFrame(5, image));
    QCOMPARE(surface.hasFrame(), true);
    QCOMPARE(surface.activeFrameFormat(), VideoSurface::RgbaFrame);
    QVERIFY(surface.currentSerial() > 0);
    const quint64 rgbaSerial = surface.currentSerial();
    QCOMPARE(frameSpy.count(), 1);

    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 2;
    frame.height = 2;
    frame.yStride = 2;
    frame.uStride = 1;
    frame.vStride = 1;
    frame.yPlane = QByteArray(4, char(235));
    frame.uPlane = QByteArray(1, char(128));
    frame.vPlane = QByteArray(1, char(128));

    QVERIFY(renderer.pushYuv420Frame(5, frame));
    QCOMPARE(surface.hasFrame(), true);
    QCOMPARE(surface.activeFrameFormat(), VideoSurface::Yuv420Frame);
    QVERIFY(surface.currentSerial() > rgbaSerial);
    QCOMPARE(frameSpy.count(), 2);

    QVERIFY(renderer.pushFrame(5, image));
    QCOMPARE(surface.hasFrame(), true);
    QCOMPARE(surface.activeFrameFormat(), VideoSurface::RgbaFrame);
    QCOMPARE(frameSpy.count(), 3);

    renderer.clearChannel(5);
    QCOMPARE(surface.hasFrame(), false);
    QCOMPARE(surface.activeFrameFormat(), VideoSurface::NoFrame);
    QCOMPARE(surface.currentSerial(), quint64(0));
    QCOMPARE(frameSpy.count(), 4);
}

void VideoSurfaceTest::createsYuvRenderNodeForYuvFrames()
{
    GlobalVideoRenderer renderer;
    VideoSurface surface;
    surface.setWidth(160);
    surface.setHeight(90);
    surface.setChannelId(7);
    surface.setVideoSink(&renderer);

    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 2;
    frame.height = 2;
    frame.yStride = 2;
    frame.uStride = 1;
    frame.vStride = 1;
    frame.yPlane = QByteArray(4, char(235));
    frame.uPlane = QByteArray(1, char(128));
    frame.vPlane = QByteArray(1, char(128));

    QVERIFY(renderer.pushYuv420Frame(7, frame));
    QCOMPARE(surface.activeFrameFormat(), VideoSurface::Yuv420Frame);

    QSGNode* node = surface.updatePaintNode(nullptr, nullptr);
    QVERIFY(node != nullptr);

    auto* yuvNode = dynamic_cast<Yuv420RenderNode*>(node);
    if (!yuvNode && node->childCount() > 0)
        yuvNode = dynamic_cast<Yuv420RenderNode*>(node->childAtIndex(0));

    QVERIFY(yuvNode != nullptr);
    QCOMPARE(yuvNode->serial(), surface.currentSerial());
    QCOMPARE(yuvNode->rect(), QRectF(35, 0, 90, 90));

    delete node;
}

QTEST_MAIN(VideoSurfaceTest)

#include "tst_video_surface.moc"

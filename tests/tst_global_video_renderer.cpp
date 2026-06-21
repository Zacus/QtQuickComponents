#include <QtTest/QtTest>

#include <QByteArray>

#include <thread>

#include "GlobalVideoRenderer.h"

using QuickUI::Components::Internal::GlobalVideoRenderer;

class GlobalVideoRendererTest : public QObject
{
    Q_OBJECT

private slots:
    void storesIndependentChannelFrames();
    void clearChannelRemovesOnlyThatChannel();
    void pushFrameCanRunFromWorkerThread();
    void pushYuv420FrameConvertsToRgbaSnapshot();
    void pushYuv420FrameRejectsInvalidInput();
    void pushYuv420FrameStoresPlaneSnapshotCopy();
    void pushFrameReplacesYuvSnapshotForChannel();
};

static QByteArray bytes(std::initializer_list<unsigned char> values)
{
    QByteArray data;
    data.reserve(static_cast<qsizetype>(values.size()));
    for (const unsigned char value : values)
        data.append(static_cast<char>(value));
    return data;
}

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

    QVERIFY(renderer.pushFrame(1, image));
    QVERIFY(renderer.pushFrame(2, image));
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

void GlobalVideoRendererTest::pushYuv420FrameConvertsToRgbaSnapshot()
{
    GlobalVideoRenderer renderer;
    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 2;
    frame.height = 2;
    frame.yStride = 2;
    frame.uStride = 1;
    frame.vStride = 1;
    frame.yPlane = bytes({235, 235, 235, 235});
    frame.uPlane = bytes({128});
    frame.vPlane = bytes({128});

    QVERIFY(renderer.pushYuv420Frame(11, frame));

    const auto snapshot = renderer.frameSnapshot(11);
    QVERIFY(snapshot.isValid());
    QCOMPARE(snapshot.image.size(), QSize(2, 2));
    QCOMPARE(snapshot.image.pixelColor(0, 0), QColor(255, 255, 255));
    QCOMPARE(snapshot.image.pixelColor(1, 1), QColor(255, 255, 255));
}

void GlobalVideoRendererTest::pushYuv420FrameRejectsInvalidInput()
{
    GlobalVideoRenderer renderer;
    GlobalVideoRenderer::Yuv420Frame oddSize;
    oddSize.width = 3;
    oddSize.height = 2;
    oddSize.yStride = 3;
    oddSize.uStride = 2;
    oddSize.vStride = 2;
    oddSize.yPlane = bytes({16, 16, 16, 16, 16, 16});
    oddSize.uPlane = bytes({128, 128});
    oddSize.vPlane = bytes({128, 128});

    QVERIFY(!renderer.pushYuv420Frame(3, oddSize));
    QVERIFY(!renderer.frameSnapshot(3).isValid());

    GlobalVideoRenderer::Yuv420Frame shortPlane;
    shortPlane.width = 2;
    shortPlane.height = 2;
    shortPlane.yStride = 2;
    shortPlane.uStride = 1;
    shortPlane.vStride = 1;
    shortPlane.yPlane = bytes({16, 16, 16});
    shortPlane.uPlane = bytes({128});
    shortPlane.vPlane = bytes({128});

    QVERIFY(!renderer.pushYuv420Frame(4, shortPlane));
    QVERIFY(!renderer.frameSnapshot(4).isValid());
}

void GlobalVideoRendererTest::pushYuv420FrameStoresPlaneSnapshotCopy()
{
    GlobalVideoRenderer renderer;
    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 2;
    frame.height = 2;
    frame.yStride = 4;
    frame.uStride = 2;
    frame.vStride = 2;
    frame.yPlane = bytes({16, 32, 0, 0, 48, 64});
    frame.uPlane = bytes({127, 0});
    frame.vPlane = bytes({129, 0});
    char* yData = frame.yPlane.data();
    char* uData = frame.uPlane.data();
    char* vData = frame.vPlane.data();

    QVERIFY(renderer.pushYuv420Frame(21, frame));
    yData[0] = static_cast<char>(235);
    uData[0] = static_cast<char>(16);
    vData[0] = static_cast<char>(240);

    const auto snapshot = renderer.yuv420Snapshot(21);
    QVERIFY(snapshot.isValid());
    QCOMPARE(snapshot.frame.width, 2);
    QCOMPARE(snapshot.frame.height, 2);
    QCOMPARE(snapshot.frame.yStride, 4);
    QCOMPARE(snapshot.frame.yPlane.at(0), char(16));
    QCOMPARE(snapshot.frame.uPlane.at(0), char(127));
    QCOMPARE(snapshot.frame.vPlane.at(0), char(129));
    QCOMPARE(snapshot.serial, renderer.frameSerial(21));

    renderer.clearChannel(21);
    QVERIFY(!renderer.yuv420Snapshot(21).isValid());
    QVERIFY(!renderer.frameSnapshot(21).isValid());
}

void GlobalVideoRendererTest::pushFrameReplacesYuvSnapshotForChannel()
{
    GlobalVideoRenderer renderer;
    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 2;
    frame.height = 2;
    frame.yStride = 2;
    frame.uStride = 1;
    frame.vStride = 1;
    frame.yPlane = bytes({235, 235, 235, 235});
    frame.uPlane = bytes({128});
    frame.vPlane = bytes({128});

    QVERIFY(renderer.pushYuv420Frame(22, frame));
    QVERIFY(renderer.yuv420Snapshot(22).isValid());

    QImage image(2, 2, QImage::Format_RGBA8888);
    image.fill(Qt::black);
    QVERIFY(renderer.pushFrame(22, image));

    QVERIFY(!renderer.yuv420Snapshot(22).isValid());
    QVERIFY(renderer.frameSnapshot(22).isValid());
    QCOMPARE(renderer.frameSnapshot(22).image.pixelColor(0, 0), QColor(Qt::black));
}

QTEST_APPLESS_MAIN(GlobalVideoRendererTest)

#include "tst_global_video_renderer.moc"

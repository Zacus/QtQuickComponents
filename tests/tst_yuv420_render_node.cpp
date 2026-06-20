#include <QtTest/QtTest>

#include "GlobalVideoRenderer.h"
#include "Yuv420RenderNode.h"

#include <rhi/qrhi_platform.h>

#include <memory>

namespace {

struct ResourceUpdateBatchReleaser
{
    void operator()(QRhiResourceUpdateBatch* updates) const
    {
        if (updates)
            updates->release();
    }
};

using ResourceUpdateBatchPtr = std::unique_ptr<QRhiResourceUpdateBatch, ResourceUpdateBatchReleaser>;

} // namespace

class Yuv420RenderNodeTest : public QObject
{
    Q_OBJECT

private slots:
    void storesSnapshotAndBounds();
    void tracksTextureUploadStateForFrames();
    void releasesTextureResourcesWithSceneGraphNode();
    void uploadsCurrentSnapshotToTextureResources();
};

void Yuv420RenderNodeTest::storesSnapshotAndBounds()
{
    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 42;
    snapshot.frame.width = 2;
    snapshot.frame.height = 2;
    snapshot.frame.yStride = 2;
    snapshot.frame.uStride = 1;
    snapshot.frame.vStride = 1;
    snapshot.frame.yPlane = QByteArray(4, char(16));
    snapshot.frame.uPlane = QByteArray(1, char(128));
    snapshot.frame.vPlane = QByteArray(1, char(128));

    Yuv420RenderNode node(snapshot, QRectF(1, 2, 30, 40));
    snapshot.frame.yPlane[0] = char(235);

    QCOMPARE(node.serial(), quint64(42));
    QCOMPARE(node.rect(), QRectF(1, 2, 30, 40));
    QVERIFY(node.flags().testFlag(QSGRenderNode::BoundedRectRendering));
    QCOMPARE(node.snapshot().frame.yPlane.at(0), char(16));

    node.setRect(QRectF(4, 5, 60, 70));
    QCOMPARE(node.rect(), QRectF(4, 5, 60, 70));

    node.render(nullptr);
}

void Yuv420RenderNodeTest::tracksTextureUploadStateForFrames()
{
    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 12;
    snapshot.frame.width = 4;
    snapshot.frame.height = 6;
    snapshot.frame.yStride = 8;
    snapshot.frame.uStride = 4;
    snapshot.frame.vStride = 4;
    snapshot.frame.yPlane = QByteArray(46, char(16));
    snapshot.frame.uPlane = QByteArray(14, char(128));
    snapshot.frame.vPlane = QByteArray(14, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 60));

    QCOMPARE(node.yTextureSize(), QSize(4, 6));
    QCOMPARE(node.uTextureSize(), QSize(2, 3));
    QCOMPARE(node.vTextureSize(), QSize(2, 3));
    QCOMPARE(node.uploadedSerial(), quint64(0));
    QVERIFY(node.hasPendingTextureUpload());

    QVERIFY(node.markTextureUploadCompleteForCurrentSnapshot());
    QCOMPARE(node.uploadedSerial(), quint64(12));
    QVERIFY(!node.hasPendingTextureUpload());

    node.setSnapshot(snapshot);
    QVERIFY(!node.hasPendingTextureUpload());

    snapshot.serial = 13;
    node.setSnapshot(snapshot);
    QVERIFY(node.hasPendingTextureUpload());
    QCOMPARE(node.uploadedSerial(), quint64(12));
}

void Yuv420RenderNodeTest::releasesTextureResourcesWithSceneGraphNode()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 21;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QVERIFY(node.ensureTextureResources(rhi.get()));
    QVERIFY(node.hasTextureResources());

    node.releaseResources();
    QVERIFY(!node.hasTextureResources());
}

void Yuv420RenderNodeTest::uploadsCurrentSnapshotToTextureResources()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 32;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 8;
    snapshot.frame.uStride = 4;
    snapshot.frame.vStride = 4;
    snapshot.frame.yPlane = QByteArray(28, char(16));
    snapshot.frame.uPlane = QByteArray(6, char(128));
    snapshot.frame.vPlane = QByteArray(6, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QVERIFY(node.hasPendingTextureUpload());

    QVERIFY(node.uploadPendingTextureData(rhi.get(), updates.get()));
    QVERIFY(node.hasTextureResources());
    QCOMPARE(node.uploadedSerial(), quint64(32));
    QVERIFY(!node.hasPendingTextureUpload());

    QVERIFY(node.ensureShaderResources(rhi.get()));
    QVERIFY(node.hasShaderResources());

    node.releaseResources();
    QVERIFY(!node.hasTextureResources());
    QVERIFY(!node.hasShaderResources());
}

QTEST_APPLESS_MAIN(Yuv420RenderNodeTest)

#include "tst_yuv420_render_node.moc"

#include <QtTest/QtTest>

#include "GlobalVideoRenderer.h"
#include "Yuv420RenderNode.h"

class Yuv420RenderNodeTest : public QObject
{
    Q_OBJECT

private slots:
    void storesSnapshotAndBounds();
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

QTEST_APPLESS_MAIN(Yuv420RenderNodeTest)

#include "tst_yuv420_render_node.moc"

#include <QtTest/QtTest>

#include "Yuv420GeometryBuffer.h"
#include "Yuv420ShaderPipeline.h"

#include <rhi/qrhi_platform.h>

#include <memory>

using QuickUI::Components::Internal::Yuv420GeometryBuffer;
using QuickUI::Components::Internal::Yuv420ShaderPipeline;

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

class Yuv420GeometryBufferTest : public QObject
{
    Q_OBJECT

private slots:
    void createsUploadsAndReusesStaticVertexBuffer();
    void recreatesVertexBufferWhenVerticesChange();
    void rejectsInvalidInputs();
};

void Yuv420GeometryBufferTest::createsUploadsAndReusesStaticVertexBuffer()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    Yuv420GeometryBuffer geometry;
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    QVERIFY(geometry.ensure(rhi.get(), updates.get()));
    QVERIFY(geometry.isValid());
    QCOMPARE(geometry.rhi(), rhi.get());
    QCOMPARE(geometry.vertexCount(), quint32(Yuv420ShaderPipeline::quadVertices().size()));
    QVERIFY(geometry.vertexBuffer() != nullptr);
    QCOMPARE(geometry.vertexBuffer()->type(), QRhiBuffer::Immutable);
    QCOMPARE(geometry.vertexBuffer()->usage(), QRhiBuffer::UsageFlags(QRhiBuffer::VertexBuffer));
    QCOMPARE(geometry.vertexBuffer()->size(),
             quint32(Yuv420ShaderPipeline::quadVertices().size() * sizeof(Yuv420ShaderPipeline::Vertex)));

    QCOMPARE(geometry.vertexOffset(), quint32(0));
    QCOMPARE(geometry.vertices().size(), Yuv420ShaderPipeline::quadVertices().size());

    QRhiBuffer* buffer = geometry.vertexBuffer();
    QVERIFY(geometry.ensure(rhi.get(), updates.get()));
    QCOMPARE(geometry.vertexBuffer(), buffer);

    geometry.release();
    QVERIFY(!geometry.isValid());
    QCOMPARE(geometry.rhi(), nullptr);
    QCOMPARE(geometry.vertexBuffer(), nullptr);
    QCOMPARE(geometry.vertexCount(), quint32(0));
    QVERIFY(geometry.vertices().isEmpty());
}

void Yuv420GeometryBufferTest::recreatesVertexBufferWhenVerticesChange()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    Yuv420GeometryBuffer geometry;
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    const QVector<Yuv420ShaderPipeline::Vertex> firstVertices =
        Yuv420ShaderPipeline::quadVertices(QRectF(0, 0, 160, 90));
    QVERIFY(geometry.ensure(rhi.get(), updates.get(), firstVertices));
    QCOMPARE(geometry.vertexCount(), quint32(firstVertices.size()));
    QCOMPARE(geometry.vertices().at(0).x, 0.0f);
    QCOMPARE(geometry.vertices().at(0).y, 90.0f);

    const QVector<Yuv420ShaderPipeline::Vertex> resizedVertices =
        Yuv420ShaderPipeline::quadVertices(QRectF(0, 0, 320, 180));
    QVERIFY(geometry.ensure(rhi.get(), updates.get(), resizedVertices));
    QCOMPARE(geometry.vertexCount(), quint32(resizedVertices.size()));
    QCOMPARE(geometry.vertices().at(0).x, 0.0f);
    QCOMPARE(geometry.vertices().at(0).y, 180.0f);
    QCOMPARE(geometry.vertices().at(3).x, 320.0f);
    QCOMPARE(geometry.vertices().at(3).y, 0.0f);
}

void Yuv420GeometryBufferTest::rejectsInvalidInputs()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    Yuv420GeometryBuffer geometry;
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    QVERIFY(!geometry.ensure(nullptr, updates.get()));
    QVERIFY(!geometry.ensure(rhi.get(), nullptr));
    QVERIFY(!geometry.isValid());
}

QTEST_APPLESS_MAIN(Yuv420GeometryBufferTest)

#include "tst_yuv420_geometry_buffer.moc"

#include <QtTest/QtTest>

#include "GlobalVideoRenderer.h"
#include "Yuv420RenderNode.h"

#include <rhi/qrhi_platform.h>

#include <memory>

using QuickUI::Components::Internal::GlobalVideoRenderer;
using QuickUI::Components::Internal::Yuv420RenderNode;

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
    void uploadsShaderUniformsWithSceneGraphNode();
    void ownsGeometryBufferForDraw();
    void ownsGraphicsPipelineForDraw();
    void preparesAllResourcesForDraw();
    void recordsDrawCommandsForPreparedResources();
    void tracksRhiAndReleasesResourcesWhenRhiChanges();
    void rendersFrameWithSceneGraphResources();
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

    QVERIFY(node.uploadShaderUniforms(rhi.get(), updates.get(), 0.8f));
    QVERIFY(node.ensureShaderResources(rhi.get()));
    QVERIFY(node.hasShaderResources());

    node.releaseResources();
    QVERIFY(!node.hasTextureResources());
    QVERIFY(!node.hasShaderResources());
}

void Yuv420RenderNodeTest::uploadsShaderUniformsWithSceneGraphNode()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 44;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QVERIFY(node.uploadShaderUniforms(rhi.get(), updates.get(), 0.6f));
    QVERIFY(node.hasShaderUniforms());

    node.releaseResources();
    QVERIFY(!node.hasShaderUniforms());
}

void Yuv420RenderNodeTest::ownsGeometryBufferForDraw()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 19;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    QVERIFY(!node.hasGeometryResources());
    QVERIFY(node.ensureGeometryResources(rhi.get(), updates.get()));
    QVERIFY(node.hasGeometryResources());
    QVERIFY(node.geometryBuffer() != nullptr);
    QCOMPARE(node.geometryBuffer()->usage(), QRhiBuffer::UsageFlags(QRhiBuffer::VertexBuffer));

    QRhiBuffer* buffer = node.geometryBuffer();
    QVERIFY(node.ensureGeometryResources(rhi.get(), updates.get()));
    QCOMPARE(node.geometryBuffer(), buffer);

    node.releaseResources();
    QVERIFY(!node.hasGeometryResources());
    QCOMPARE(node.geometryBuffer(), nullptr);
}

void Yuv420RenderNodeTest::ownsGraphicsPipelineForDraw()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    std::unique_ptr<QRhiTexture> colorTexture(rhi->newTexture(QRhiTexture::RGBA8,
                                                              QSize(4, 4),
                                                              1,
                                                              QRhiTexture::RenderTarget));
    QVERIFY(colorTexture != nullptr);
    QVERIFY(colorTexture->create());

    QRhiTextureRenderTargetDescription renderTargetDescription(QRhiColorAttachment(colorTexture.get()));
    std::unique_ptr<QRhiTextureRenderTarget> renderTarget(rhi->newTextureRenderTarget(renderTargetDescription));
    QVERIFY(renderTarget != nullptr);
    std::unique_ptr<QRhiRenderPassDescriptor> renderPass(renderTarget->newCompatibleRenderPassDescriptor());
    QVERIFY(renderPass != nullptr);
    renderTarget->setRenderPassDescriptor(renderPass.get());
    QVERIFY(renderTarget->create());

    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 27;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QVERIFY(!node.ensurePipelineResources(rhi.get(), renderPass.get()));
    QVERIFY(!node.hasPipelineResources());

    QVERIFY(node.uploadPendingTextureData(rhi.get(), updates.get()));
    QVERIFY(node.uploadShaderUniforms(rhi.get(), updates.get(), 1.0f));
    QVERIFY(node.ensureShaderResources(rhi.get()));
    QVERIFY(node.ensurePipelineResources(rhi.get(), renderPass.get()));
    QVERIFY(node.hasPipelineResources());
    QVERIFY(node.graphicsPipeline() != nullptr);

    QRhiGraphicsPipeline* pipeline = node.graphicsPipeline();
    QVERIFY(node.ensurePipelineResources(rhi.get(), renderPass.get()));
    QCOMPARE(node.graphicsPipeline(), pipeline);

    node.releaseResources();
    QVERIFY(!node.hasPipelineResources());
    QCOMPARE(node.graphicsPipeline(), nullptr);
}

void Yuv420RenderNodeTest::preparesAllResourcesForDraw()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    std::unique_ptr<QRhiTexture> colorTexture(rhi->newTexture(QRhiTexture::RGBA8,
                                                              QSize(4, 4),
                                                              1,
                                                              QRhiTexture::RenderTarget));
    QVERIFY(colorTexture != nullptr);
    QVERIFY(colorTexture->create());

    QRhiTextureRenderTargetDescription renderTargetDescription(QRhiColorAttachment(colorTexture.get()));
    std::unique_ptr<QRhiTextureRenderTarget> renderTarget(rhi->newTextureRenderTarget(renderTargetDescription));
    QVERIFY(renderTarget != nullptr);
    std::unique_ptr<QRhiRenderPassDescriptor> renderPass(renderTarget->newCompatibleRenderPassDescriptor());
    QVERIFY(renderPass != nullptr);
    renderTarget->setRenderPassDescriptor(renderPass.get());
    QVERIFY(renderTarget->create());

    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 35;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QVERIFY(!node.prepareResources(rhi.get(), updates.get(), nullptr, 0.75f));
    QVERIFY(node.hasPendingTextureUpload());

    QVERIFY(node.prepareResources(rhi.get(), updates.get(), renderPass.get(), 0.75f));
    QVERIFY(!node.hasPendingTextureUpload());
    QCOMPARE(node.uploadedSerial(), quint64(35));
    QVERIFY(node.hasTextureResources());
    QVERIFY(node.hasShaderUniforms());
    QVERIFY(node.hasShaderResources());
    QVERIFY(node.hasGeometryResources());
    QVERIFY(node.hasPipelineResources());
    QVERIFY(node.geometryBuffer() != nullptr);
    QVERIFY(node.graphicsPipeline() != nullptr);
}

void Yuv420RenderNodeTest::recordsDrawCommandsForPreparedResources()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    std::unique_ptr<QRhiTexture> colorTexture(rhi->newTexture(QRhiTexture::RGBA8,
                                                              QSize(4, 4),
                                                              1,
                                                              QRhiTexture::RenderTarget));
    QVERIFY(colorTexture != nullptr);
    QVERIFY(colorTexture->create());

    QRhiTextureRenderTargetDescription renderTargetDescription(QRhiColorAttachment(colorTexture.get()));
    std::unique_ptr<QRhiTextureRenderTarget> renderTarget(rhi->newTextureRenderTarget(renderTargetDescription));
    QVERIFY(renderTarget != nullptr);
    std::unique_ptr<QRhiRenderPassDescriptor> renderPass(renderTarget->newCompatibleRenderPassDescriptor());
    QVERIFY(renderPass != nullptr);
    renderTarget->setRenderPassDescriptor(renderPass.get());
    QVERIFY(renderTarget->create());

    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 41;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QVERIFY(!node.recordDrawCommands(nullptr));

    QVERIFY(node.prepareResources(rhi.get(), updates.get(), renderPass.get(), 1.0f));

    QRhiCommandBuffer* commandBuffer = nullptr;
    QCOMPARE(rhi->beginOffscreenFrame(&commandBuffer), QRhi::FrameOpSuccess);
    QVERIFY(commandBuffer != nullptr);

    commandBuffer->beginPass(renderTarget.get(), QColor(Qt::transparent), QRhiDepthStencilClearValue(), nullptr);
    QVERIFY(node.recordDrawCommands(commandBuffer));
    commandBuffer->endPass();
    QCOMPARE(rhi->endOffscreenFrame(), QRhi::FrameOpSuccess);
}

void Yuv420RenderNodeTest::tracksRhiAndReleasesResourcesWhenRhiChanges()
{
    QRhiNullInitParams params1;
    std::unique_ptr<QRhi> rhi1(QRhi::create(QRhi::Null, &params1));
    QVERIFY(rhi1 != nullptr);

    QRhiNullInitParams params2;
    std::unique_ptr<QRhi> rhi2(QRhi::create(QRhi::Null, &params2));
    QVERIFY(rhi2 != nullptr);

    std::unique_ptr<QRhiTexture> colorTexture(rhi1->newTexture(QRhiTexture::RGBA8,
                                                               QSize(4, 4),
                                                               1,
                                                               QRhiTexture::RenderTarget));
    QVERIFY(colorTexture != nullptr);
    QVERIFY(colorTexture->create());

    QRhiTextureRenderTargetDescription renderTargetDescription(QRhiColorAttachment(colorTexture.get()));
    std::unique_ptr<QRhiTextureRenderTarget> renderTarget(rhi1->newTextureRenderTarget(renderTargetDescription));
    QVERIFY(renderTarget != nullptr);
    std::unique_ptr<QRhiRenderPassDescriptor> renderPass(renderTarget->newCompatibleRenderPassDescriptor());
    QVERIFY(renderPass != nullptr);
    renderTarget->setRenderPassDescriptor(renderPass.get());
    QVERIFY(renderTarget->create());

    ResourceUpdateBatchPtr updates(rhi1->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 47;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QCOMPARE(node.rhi(), nullptr);

    node.setRhi(rhi1.get());
    QCOMPARE(node.rhi(), rhi1.get());
    QVERIFY(node.prepareResources(rhi1.get(), updates.get(), renderPass.get(), 1.0f));
    QVERIFY(node.hasTextureResources());
    QVERIFY(node.hasShaderUniforms());
    QVERIFY(node.hasShaderResources());
    QVERIFY(node.hasGeometryResources());
    QVERIFY(node.hasPipelineResources());

    node.setRhi(rhi1.get());
    QVERIFY(node.hasTextureResources());

    node.setRhi(rhi2.get());
    QCOMPARE(node.rhi(), rhi2.get());
    QVERIFY(!node.hasTextureResources());
    QVERIFY(!node.hasShaderUniforms());
    QVERIFY(!node.hasShaderResources());
    QVERIFY(!node.hasGeometryResources());
    QVERIFY(!node.hasPipelineResources());
}

void Yuv420RenderNodeTest::rendersFrameWithSceneGraphResources()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    std::unique_ptr<QRhiTexture> colorTexture(rhi->newTexture(QRhiTexture::RGBA8,
                                                              QSize(4, 4),
                                                              1,
                                                              QRhiTexture::RenderTarget));
    QVERIFY(colorTexture != nullptr);
    QVERIFY(colorTexture->create());

    QRhiTextureRenderTargetDescription renderTargetDescription(QRhiColorAttachment(colorTexture.get()));
    std::unique_ptr<QRhiTextureRenderTarget> renderTarget(rhi->newTextureRenderTarget(renderTargetDescription));
    QVERIFY(renderTarget != nullptr);
    std::unique_ptr<QRhiRenderPassDescriptor> renderPass(renderTarget->newCompatibleRenderPassDescriptor());
    QVERIFY(renderPass != nullptr);
    renderTarget->setRenderPassDescriptor(renderPass.get());
    QVERIFY(renderTarget->create());

    GlobalVideoRenderer::Yuv420Snapshot snapshot;
    snapshot.serial = 53;
    snapshot.frame.width = 4;
    snapshot.frame.height = 4;
    snapshot.frame.yStride = 4;
    snapshot.frame.uStride = 2;
    snapshot.frame.vStride = 2;
    snapshot.frame.yPlane = QByteArray(16, char(16));
    snapshot.frame.uPlane = QByteArray(4, char(128));
    snapshot.frame.vPlane = QByteArray(4, char(128));

    Yuv420RenderNode node(snapshot, QRectF(0, 0, 40, 40));
    QVERIFY(!node.renderFrame(renderTarget.get(), nullptr, 1.0f));
    node.setRhi(rhi.get());

    QRhiCommandBuffer* commandBuffer = nullptr;
    QCOMPARE(rhi->beginOffscreenFrame(&commandBuffer), QRhi::FrameOpSuccess);
    QVERIFY(commandBuffer != nullptr);

    commandBuffer->beginPass(renderTarget.get(), QColor(Qt::transparent), QRhiDepthStencilClearValue(), nullptr);
    QVERIFY(node.renderFrame(renderTarget.get(), commandBuffer, 0.5f));
    commandBuffer->endPass();
    QCOMPARE(rhi->endOffscreenFrame(), QRhi::FrameOpSuccess);

    QVERIFY(!node.hasPendingTextureUpload());
    QVERIFY(node.hasTextureResources());
    QVERIFY(node.hasShaderUniforms());
    QVERIFY(node.hasShaderResources());
    QVERIFY(node.hasGeometryResources());
    QVERIFY(node.hasPipelineResources());
}

QTEST_APPLESS_MAIN(Yuv420RenderNodeTest)

#include "tst_yuv420_render_node.moc"

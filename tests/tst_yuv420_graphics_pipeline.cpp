#include <QtTest/QtTest>

#include "GlobalVideoRenderer.h"
#include "Yuv420GraphicsPipeline.h"
#include "Yuv420ShaderUniforms.h"
#include "Yuv420TextureSet.h"

#include <rhi/qrhi_platform.h>

#include <memory>

using QuickUI::Components::Internal::GlobalVideoRenderer;
using QuickUI::Components::Internal::Yuv420GraphicsPipeline;
using QuickUI::Components::Internal::Yuv420ShaderUniforms;
using QuickUI::Components::Internal::Yuv420TextureSet;

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

GlobalVideoRenderer::Yuv420Frame makeFrame()
{
    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 4;
    frame.height = 4;
    frame.yStride = 4;
    frame.uStride = 2;
    frame.vStride = 2;
    frame.yPlane = QByteArray(16, char(16));
    frame.uPlane = QByteArray(4, char(128));
    frame.vPlane = QByteArray(4, char(128));
    return frame;
}

} // namespace

class Yuv420GraphicsPipelineTest : public QObject
{
    Q_OBJECT

private slots:
    void createsReusesAndReleasesPipeline();
    void rejectsMissingDependencies();
};

void Yuv420GraphicsPipelineTest::createsReusesAndReleasesPipeline()
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

    Yuv420TextureSet textures;
    Yuv420ShaderUniforms uniforms;
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);
    QVERIFY(textures.uploadFrame(rhi.get(), updates.get(), makeFrame(), 31));
    QVERIFY(uniforms.upload(rhi.get(), updates.get(), 1.0f, 31));
    QVERIFY(textures.ensureShaderResources(rhi.get(), uniforms.buffer()));

    Yuv420GraphicsPipeline pipeline;
    QVERIFY(pipeline.ensure(rhi.get(), textures.shaderResourceBindings(), renderPass.get()));
    QVERIFY(pipeline.isValid());
    QCOMPARE(pipeline.rhi(), rhi.get());
    QCOMPARE(pipeline.shaderResourceBindings(), textures.shaderResourceBindings());
    QCOMPARE(pipeline.renderPassDescriptor(), renderPass.get());
    QVERIFY(pipeline.graphicsPipeline() != nullptr);
    QCOMPARE(pipeline.graphicsPipeline()->topology(), QRhiGraphicsPipeline::TriangleStrip);
    QCOMPARE(pipeline.graphicsPipeline()->shaderStageCount(), qsizetype(2));
    QCOMPARE(pipeline.graphicsPipeline()->vertexInputLayout().attributeCount(), qsizetype(2));
    QCOMPARE(pipeline.graphicsPipeline()->targetBlendCount(), qsizetype(1));
    QVERIFY(pipeline.graphicsPipeline()->targetBlendAt(0)->enable);
    QCOMPARE(pipeline.graphicsPipeline()->targetBlendAt(0)->srcColor, QRhiGraphicsPipeline::SrcAlpha);
    QCOMPARE(pipeline.graphicsPipeline()->targetBlendAt(0)->dstColor, QRhiGraphicsPipeline::OneMinusSrcAlpha);

    QRhiGraphicsPipeline* graphicsPipeline = pipeline.graphicsPipeline();
    QVERIFY(pipeline.ensure(rhi.get(), textures.shaderResourceBindings(), renderPass.get()));
    QCOMPARE(pipeline.graphicsPipeline(), graphicsPipeline);

    pipeline.release();
    QVERIFY(!pipeline.isValid());
    QCOMPARE(pipeline.rhi(), nullptr);
    QCOMPARE(pipeline.graphicsPipeline(), nullptr);
}

void Yuv420GraphicsPipelineTest::rejectsMissingDependencies()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    Yuv420GraphicsPipeline pipeline;
    QVERIFY(!pipeline.ensure(nullptr, nullptr, nullptr));
    QVERIFY(!pipeline.ensure(rhi.get(), nullptr, nullptr));
    QVERIFY(!pipeline.isValid());
}

QTEST_APPLESS_MAIN(Yuv420GraphicsPipelineTest)

#include "tst_yuv420_graphics_pipeline.moc"

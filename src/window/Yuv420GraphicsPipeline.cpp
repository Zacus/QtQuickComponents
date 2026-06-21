#include "Yuv420GraphicsPipeline.h"

#include "Yuv420ShaderPipeline.h"

#include <rhi/qrhi.h>

void Yuv420GraphicsPipeline::PipelineDeleter::operator()(QRhiGraphicsPipeline* pipeline) const
{
    delete pipeline;
}

Yuv420GraphicsPipeline::~Yuv420GraphicsPipeline()
{
    release();
}

bool Yuv420GraphicsPipeline::ensure(QRhi* rhi,
                                    QRhiShaderResourceBindings* shaderResourceBindings,
                                    QRhiRenderPassDescriptor* renderPassDescriptor)
{
    if (!rhi || !shaderResourceBindings || !renderPassDescriptor)
        return false;

    if (m_rhi != rhi
        || m_shaderResourceBindings != shaderResourceBindings
        || m_renderPassDescriptor != renderPassDescriptor) {
        release();
        m_rhi = rhi;
        m_shaderResourceBindings = shaderResourceBindings;
        m_renderPassDescriptor = renderPassDescriptor;
    }

    if (m_pipeline)
        return true;

    Yuv420ShaderPipeline shaderPipeline;
    if (!shaderPipeline.loadShaders())
        return false;

    std::unique_ptr<QRhiGraphicsPipeline, PipelineDeleter> pipeline(m_rhi->newGraphicsPipeline());
    if (!pipeline)
        return false;

    QRhiGraphicsPipeline::TargetBlend targetBlend;
    targetBlend.enable = true;
    targetBlend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
    targetBlend.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    targetBlend.srcAlpha = QRhiGraphicsPipeline::One;
    targetBlend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;

    const QVector<QRhiShaderStage> shaderStages = shaderPipeline.shaderStages();
    pipeline->setTopology(Yuv420ShaderPipeline::topology());
    pipeline->setShaderStages(shaderStages.cbegin(), shaderStages.cend());
    pipeline->setVertexInputLayout(Yuv420ShaderPipeline::vertexInputLayout());
    pipeline->setShaderResourceBindings(m_shaderResourceBindings);
    pipeline->setRenderPassDescriptor(m_renderPassDescriptor);
    pipeline->setTargetBlends({targetBlend});

    if (!pipeline->create())
        return false;

    m_pipeline = std::move(pipeline);
    return true;
}

void Yuv420GraphicsPipeline::release()
{
    if (m_pipeline) {
        m_pipeline->destroy();
        m_pipeline.reset();
    }

    m_rhi = nullptr;
    m_shaderResourceBindings = nullptr;
    m_renderPassDescriptor = nullptr;
}

bool Yuv420GraphicsPipeline::isValid() const
{
    return m_rhi && m_shaderResourceBindings && m_renderPassDescriptor && m_pipeline;
}

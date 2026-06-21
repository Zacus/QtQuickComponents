#pragma once

#include <memory>

class QRhi;
class QRhiGraphicsPipeline;
class QRhiRenderPassDescriptor;
class QRhiShaderResourceBindings;

namespace QuickUI::Components::Internal {

class Yuv420GraphicsPipeline
{
public:
    Yuv420GraphicsPipeline() = default;
    ~Yuv420GraphicsPipeline();

    bool ensure(QRhi* rhi,
                QRhiShaderResourceBindings* shaderResourceBindings,
                QRhiRenderPassDescriptor* renderPassDescriptor);
    void release();

    bool isValid() const;
    QRhi* rhi() const { return m_rhi; }
    QRhiGraphicsPipeline* graphicsPipeline() const { return m_pipeline.get(); }
    QRhiShaderResourceBindings* shaderResourceBindings() const { return m_shaderResourceBindings; }
    QRhiRenderPassDescriptor* renderPassDescriptor() const { return m_renderPassDescriptor; }

private:
    struct PipelineDeleter
    {
        void operator()(QRhiGraphicsPipeline* pipeline) const;
    };

    QRhi* m_rhi = nullptr;
    QRhiShaderResourceBindings* m_shaderResourceBindings = nullptr;
    QRhiRenderPassDescriptor* m_renderPassDescriptor = nullptr;
    std::unique_ptr<QRhiGraphicsPipeline, PipelineDeleter> m_pipeline;
};

} // namespace QuickUI::Components::Internal

#include "Yuv420RenderNode.h"

#include <rhi/qrhi.h>

namespace {

QByteArray deepCopyBytes(const QByteArray& data)
{
    return QByteArray(data.constData(), data.size());
}

GlobalVideoRenderer::Yuv420Frame deepCopyFrame(const GlobalVideoRenderer::Yuv420Frame& frame)
{
    GlobalVideoRenderer::Yuv420Frame copy;
    copy.width = frame.width;
    copy.height = frame.height;
    copy.yStride = frame.yStride;
    copy.uStride = frame.uStride;
    copy.vStride = frame.vStride;
    copy.yPlane = deepCopyBytes(frame.yPlane);
    copy.uPlane = deepCopyBytes(frame.uPlane);
    copy.vPlane = deepCopyBytes(frame.vPlane);
    return copy;
}

GlobalVideoRenderer::Yuv420Snapshot deepCopySnapshot(const GlobalVideoRenderer::Yuv420Snapshot& snapshot)
{
    GlobalVideoRenderer::Yuv420Snapshot copy;
    copy.serial = snapshot.serial;
    copy.frame = deepCopyFrame(snapshot.frame);
    return copy;
}

} // namespace

Yuv420RenderNode::Yuv420RenderNode(const GlobalVideoRenderer::Yuv420Snapshot& snapshot, const QRectF& rect)
    : m_snapshot(deepCopySnapshot(snapshot))
    , m_rect(rect)
{
    updateTexturePlan();
}

void Yuv420RenderNode::setSnapshot(const GlobalVideoRenderer::Yuv420Snapshot& snapshot)
{
    m_snapshot = deepCopySnapshot(snapshot);
    updateTexturePlan();
}

void Yuv420RenderNode::setRect(const QRectF& rect)
{
    m_rect = rect;
}

QSize Yuv420RenderNode::yTextureSize() const
{
    return m_yTextureSize;
}

QSize Yuv420RenderNode::uTextureSize() const
{
    return m_uTextureSize;
}

QSize Yuv420RenderNode::vTextureSize() const
{
    return m_vTextureSize;
}

bool Yuv420RenderNode::hasPendingTextureUpload() const
{
    return m_snapshot.isValid() && m_snapshot.serial != m_uploadedSerial;
}

bool Yuv420RenderNode::markTextureUploadCompleteForCurrentSnapshot()
{
    if (!m_snapshot.isValid())
        return false;

    m_uploadedSerial = m_snapshot.serial;
    return true;
}

bool Yuv420RenderNode::ensureTextureResources(QRhi* rhi)
{
    if (!m_snapshot.isValid())
        return false;

    return m_textures.ensureTextures(rhi, m_yTextureSize, m_uTextureSize, m_vTextureSize);
}

bool Yuv420RenderNode::hasTextureResources() const
{
    return m_textures.isValid();
}

bool Yuv420RenderNode::ensureShaderResources(QRhi* rhi)
{
    return m_textures.ensureShaderResources(rhi, m_uniforms.buffer());
}

bool Yuv420RenderNode::hasShaderResources() const
{
    return m_textures.shaderResourceBindings() != nullptr;
}

bool Yuv420RenderNode::uploadPendingTextureData(QRhi* rhi, QRhiResourceUpdateBatch* updates)
{
    if (!m_snapshot.isValid())
        return false;

    if (m_textures.uploadFrame(rhi, updates, m_snapshot.frame, m_snapshot.serial)) {
        m_uploadedSerial = m_snapshot.serial;
        return true;
    }

    return false;
}

bool Yuv420RenderNode::uploadShaderUniforms(QRhi* rhi, QRhiResourceUpdateBatch* updates, float opacity)
{
    if (!m_snapshot.isValid())
        return false;

    return m_uniforms.upload(rhi, updates, opacity, m_snapshot.serial);
}

bool Yuv420RenderNode::hasShaderUniforms() const
{
    return m_uniforms.isValid();
}

bool Yuv420RenderNode::ensureGeometryResources(QRhi* rhi, QRhiResourceUpdateBatch* updates)
{
    return m_geometry.ensure(rhi, updates);
}

bool Yuv420RenderNode::hasGeometryResources() const
{
    return m_geometry.isValid();
}

QRhiBuffer* Yuv420RenderNode::geometryBuffer() const
{
    return m_geometry.vertexBuffer();
}

bool Yuv420RenderNode::ensurePipelineResources(QRhi* rhi, QRhiRenderPassDescriptor* renderPassDescriptor)
{
    return m_pipeline.ensure(rhi, m_textures.shaderResourceBindings(), renderPassDescriptor);
}

bool Yuv420RenderNode::hasPipelineResources() const
{
    return m_pipeline.isValid();
}

QRhiGraphicsPipeline* Yuv420RenderNode::graphicsPipeline() const
{
    return m_pipeline.graphicsPipeline();
}

bool Yuv420RenderNode::prepareResources(QRhi* rhi,
                                        QRhiResourceUpdateBatch* updates,
                                        QRhiRenderPassDescriptor* renderPassDescriptor,
                                        float opacity)
{
    if (!rhi || !updates || !renderPassDescriptor || !m_snapshot.isValid())
        return false;

    return uploadPendingTextureData(rhi, updates)
        && uploadShaderUniforms(rhi, updates, opacity)
        && ensureShaderResources(rhi)
        && ensureGeometryResources(rhi, updates)
        && ensurePipelineResources(rhi, renderPassDescriptor);
}

bool Yuv420RenderNode::recordDrawCommands(QRhiCommandBuffer* commandBuffer)
{
    if (!commandBuffer
        || !hasShaderResources()
        || !hasGeometryResources()
        || !hasPipelineResources()) {
        return false;
    }

    QRhiCommandBuffer::VertexInput vertexInput(m_geometry.vertexBuffer(), m_geometry.vertexOffset());
    commandBuffer->setGraphicsPipeline(m_pipeline.graphicsPipeline());
    commandBuffer->setShaderResources(m_textures.shaderResourceBindings());
    commandBuffer->setVertexInput(0, 1, &vertexInput);
    commandBuffer->draw(m_geometry.vertexCount());
    return true;
}

QRectF Yuv420RenderNode::rect() const
{
    return m_rect;
}

QSGRenderNode::RenderingFlags Yuv420RenderNode::flags() const
{
    return BoundedRectRendering;
}

void Yuv420RenderNode::render(const RenderState*)
{
    recordDrawCommands(commandBuffer());
}

void Yuv420RenderNode::releaseResources()
{
    m_pipeline.release();
    m_geometry.release();
    m_uniforms.release();
    m_textures.release();
}

void Yuv420RenderNode::updateTexturePlan()
{
    if (!m_snapshot.isValid()) {
        m_yTextureSize = QSize();
        m_uTextureSize = QSize();
        m_vTextureSize = QSize();
        return;
    }

    const auto& frame = m_snapshot.frame;
    m_yTextureSize = QSize(frame.width, frame.height);
    m_uTextureSize = QSize(frame.width / 2, frame.height / 2);
    m_vTextureSize = QSize(frame.width / 2, frame.height / 2);
}

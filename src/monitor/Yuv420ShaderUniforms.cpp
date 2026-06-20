#include "Yuv420ShaderUniforms.h"

#include <rhi/qrhi.h>

#include <QtGlobal>

void Yuv420ShaderUniforms::BufferDeleter::operator()(QRhiBuffer* buffer) const
{
    delete buffer;
}

Yuv420ShaderUniforms::~Yuv420ShaderUniforms()
{
    release();
}

Yuv420ShaderUniforms::UniformBlock Yuv420ShaderUniforms::makeUniformBlock(float opacity)
{
    UniformBlock block;

    block.yuvToRgb[0] = 1.164383f;
    block.yuvToRgb[1] = 0.0f;
    block.yuvToRgb[2] = 1.596027f;
    block.yuvToRgb[3] = -0.874202f;

    block.yuvToRgb[4] = 1.164383f;
    block.yuvToRgb[5] = -0.391762f;
    block.yuvToRgb[6] = -0.812968f;
    block.yuvToRgb[7] = 0.531668f;

    block.yuvToRgb[8] = 1.164383f;
    block.yuvToRgb[9] = 2.017232f;
    block.yuvToRgb[10] = 0.0f;
    block.yuvToRgb[11] = -1.085631f;

    block.yuvToRgb[12] = 0.0f;
    block.yuvToRgb[13] = 0.0f;
    block.yuvToRgb[14] = 0.0f;
    block.yuvToRgb[15] = 1.0f;

    block.opacity[0] = opacity;
    return block;
}

bool Yuv420ShaderUniforms::upload(QRhi* rhi, QRhiResourceUpdateBatch* updates, float opacity, quint64 serial)
{
    if (!rhi || !updates || serial == 0)
        return false;

    if (!ensureBuffer(rhi))
        return false;

    if (m_uploadedSerial == serial && qFuzzyCompare(m_opacity, opacity))
        return true;

    m_uniformBlock = makeUniformBlock(opacity);
    updates->updateDynamicBuffer(m_buffer.get(), 0, quint32(sizeof(UniformBlock)), &m_uniformBlock);

    m_uploadedSerial = serial;
    m_opacity = opacity;
    return true;
}

void Yuv420ShaderUniforms::release()
{
    if (m_buffer) {
        m_buffer->destroy();
        m_buffer.reset();
    }

    m_rhi = nullptr;
    m_uniformBlock = UniformBlock();
    m_uploadedSerial = 0;
    m_opacity = 1.0f;
}

bool Yuv420ShaderUniforms::isValid() const
{
    return m_rhi && m_buffer;
}

bool Yuv420ShaderUniforms::ensureBuffer(QRhi* rhi)
{
    if (m_rhi != rhi) {
        release();
        m_rhi = rhi;
    }

    if (m_buffer)
        return true;

    std::unique_ptr<QRhiBuffer, BufferDeleter> buffer(
        m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, quint32(sizeof(UniformBlock))));
    if (!buffer || !buffer->create())
        return false;

    m_buffer = std::move(buffer);
    return true;
}

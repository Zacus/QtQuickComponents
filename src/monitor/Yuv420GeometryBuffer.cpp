#include "Yuv420GeometryBuffer.h"

#include "Yuv420ShaderPipeline.h"

#include <rhi/qrhi.h>

void Yuv420GeometryBuffer::BufferDeleter::operator()(QRhiBuffer* buffer) const
{
    delete buffer;
}

Yuv420GeometryBuffer::~Yuv420GeometryBuffer()
{
    release();
}

bool Yuv420GeometryBuffer::ensure(QRhi* rhi, QRhiResourceUpdateBatch* updates)
{
    if (!rhi || !updates)
        return false;

    if (m_rhi != rhi) {
        release();
        m_rhi = rhi;
    }

    if (m_vertexBuffer)
        return true;

    const QVector<Yuv420ShaderPipeline::Vertex> vertices = Yuv420ShaderPipeline::quadVertices();
    if (vertices.isEmpty())
        return false;

    const quint32 byteSize = quint32(vertices.size() * qsizetype(sizeof(Yuv420ShaderPipeline::Vertex)));
    std::unique_ptr<QRhiBuffer, BufferDeleter> buffer(
        m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, byteSize));
    if (!buffer || !buffer->create())
        return false;

    updates->uploadStaticBuffer(buffer.get(), 0, byteSize, vertices.constData());
    m_vertexCount = quint32(vertices.size());
    m_vertexBuffer = std::move(buffer);
    return true;
}

void Yuv420GeometryBuffer::release()
{
    if (m_vertexBuffer) {
        m_vertexBuffer->destroy();
        m_vertexBuffer.reset();
    }

    m_rhi = nullptr;
    m_vertexCount = 0;
}

bool Yuv420GeometryBuffer::isValid() const
{
    return m_rhi && m_vertexBuffer && m_vertexCount > 0;
}

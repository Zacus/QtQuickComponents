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
    return ensure(rhi, updates, Yuv420ShaderPipeline::quadVertices());
}

bool Yuv420GeometryBuffer::ensure(QRhi* rhi,
                                  QRhiResourceUpdateBatch* updates,
                                  const QVector<Yuv420Vertex>& vertices)
{
    if (!rhi || !updates)
        return false;

    if (m_rhi != rhi) {
        release();
        m_rhi = rhi;
    }

    if (m_vertexBuffer && m_vertices == vertices)
        return true;

    if (vertices.isEmpty())
        return false;

    if (m_vertexBuffer) {
        m_vertexBuffer->destroy();
        m_vertexBuffer.reset();
        m_vertexCount = 0;
        m_vertices.clear();
    }

    const quint32 byteSize = quint32(vertices.size() * qsizetype(sizeof(Yuv420ShaderPipeline::Vertex)));
    std::unique_ptr<QRhiBuffer, BufferDeleter> buffer(
        m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, byteSize));
    if (!buffer || !buffer->create())
        return false;

    updates->uploadStaticBuffer(buffer.get(), 0, byteSize, vertices.constData());
    m_vertexCount = quint32(vertices.size());
    m_vertices = vertices;
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
    m_vertices.clear();
    m_vertexCount = 0;
}

bool Yuv420GeometryBuffer::isValid() const
{
    return m_rhi && m_vertexBuffer && m_vertexCount > 0;
}

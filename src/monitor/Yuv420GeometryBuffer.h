#pragma once

#include <QtGlobal>
#include <memory>

class QRhi;
class QRhiBuffer;
class QRhiResourceUpdateBatch;

class Yuv420GeometryBuffer
{
public:
    Yuv420GeometryBuffer() = default;
    ~Yuv420GeometryBuffer();

    bool ensure(QRhi* rhi, QRhiResourceUpdateBatch* updates);
    void release();

    bool isValid() const;
    QRhi* rhi() const { return m_rhi; }
    QRhiBuffer* vertexBuffer() const { return m_vertexBuffer.get(); }
    quint32 vertexCount() const { return m_vertexCount; }
    quint32 vertexOffset() const { return 0; }

private:
    struct BufferDeleter
    {
        void operator()(QRhiBuffer* buffer) const;
    };

    QRhi* m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer, BufferDeleter> m_vertexBuffer;
    quint32 m_vertexCount = 0;
};

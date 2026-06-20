#pragma once

#include <QMatrix4x4>
#include <QtGlobal>

#include <array>
#include <memory>

class QRhi;
class QRhiBuffer;
class QRhiResourceUpdateBatch;

class Yuv420ShaderUniforms
{
public:
    struct UniformBlock
    {
        float transform[16] = {};
        float yuvToRgb[16] = {};
        float opacity[4] = {};
    };

    Yuv420ShaderUniforms() = default;
    ~Yuv420ShaderUniforms();

    static UniformBlock makeUniformBlock(float opacity);
    static UniformBlock makeUniformBlock(float opacity, const QMatrix4x4& transform);

    bool upload(QRhi* rhi, QRhiResourceUpdateBatch* updates, float opacity, quint64 serial);
    bool upload(QRhi* rhi,
                QRhiResourceUpdateBatch* updates,
                const QMatrix4x4& transform,
                float opacity,
                quint64 serial);
    void release();

    bool isValid() const;
    QRhiBuffer* buffer() const { return m_buffer.get(); }
    quint64 uploadedSerial() const { return m_uploadedSerial; }
    float opacity() const { return m_opacity; }
    const UniformBlock& uniformBlock() const { return m_uniformBlock; }

private:
    struct BufferDeleter
    {
        void operator()(QRhiBuffer* buffer) const;
    };

    bool ensureBuffer(QRhi* rhi);

    QRhi* m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer, BufferDeleter> m_buffer;
    UniformBlock m_uniformBlock;
    QMatrix4x4 m_transform;
    quint64 m_uploadedSerial = 0;
    float m_opacity = 1.0f;
};

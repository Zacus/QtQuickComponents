#pragma once

#include "GlobalVideoRenderer.h"

#include <QSize>

#include <memory>

class QRhi;
class QRhiBuffer;
class QRhiResourceUpdateBatch;
class QRhiSampler;
class QRhiShaderResourceBindings;
class QRhiTexture;

namespace QuickUI::Components::Internal {

class Yuv420TextureSet
{
public:
    Yuv420TextureSet() = default;
    ~Yuv420TextureSet();

    bool ensureTextures(QRhi* rhi, const QSize& ySize, const QSize& uSize, const QSize& vSize);
    bool ensureShaderResources(QRhi* rhi, QRhiBuffer* uniformBuffer);
    bool uploadFrame(QRhi* rhi,
                     QRhiResourceUpdateBatch* updates,
                     const GlobalVideoRenderer::Yuv420Frame& frame,
                     quint64 serial);
    void release();

    bool isValid() const;
    QRhi* rhi() const { return m_rhi; }
    quint64 uploadedSerial() const { return m_uploadedSerial; }

    QSize yTextureSize() const { return m_yPlane.size; }
    QSize uTextureSize() const { return m_uPlane.size; }
    QSize vTextureSize() const { return m_vPlane.size; }

    QRhiTexture* yTexture() const { return m_yPlane.texture.get(); }
    QRhiTexture* uTexture() const { return m_uPlane.texture.get(); }
    QRhiTexture* vTexture() const { return m_vPlane.texture.get(); }
    QRhiSampler* sampler() const { return m_sampler.get(); }
    QRhiShaderResourceBindings* shaderResourceBindings() const { return m_shaderResourceBindings.get(); }

private:
    struct TextureDeleter
    {
        void operator()(QRhiTexture* texture) const;
    };

    struct SamplerDeleter
    {
        void operator()(QRhiSampler* sampler) const;
    };

    struct ShaderResourceBindingsDeleter
    {
        void operator()(QRhiShaderResourceBindings* bindings) const;
    };

    struct PlaneTexture
    {
        PlaneTexture() = default;
        ~PlaneTexture();

        std::unique_ptr<QRhiTexture, TextureDeleter> texture;
        QSize size;
    };

    bool ensurePlane(PlaneTexture& plane, const QSize& size);
    bool uploadPlane(QRhiResourceUpdateBatch* updates,
                     const PlaneTexture& plane,
                     const QByteArray& bytes,
                     int stride,
                     const QSize& sourceSize);
    bool ensureSampler();
    bool ensureShaderResourceBindings(QRhiBuffer* uniformBuffer);
    void releaseShaderResourceBindings();
    void releasePlane(PlaneTexture& plane);

    QRhi* m_rhi = nullptr;
    PlaneTexture m_yPlane;
    PlaneTexture m_uPlane;
    PlaneTexture m_vPlane;
    std::unique_ptr<QRhiSampler, SamplerDeleter> m_sampler;
    std::unique_ptr<QRhiShaderResourceBindings, ShaderResourceBindingsDeleter> m_shaderResourceBindings;
    QRhiBuffer* m_uniformBuffer = nullptr;
    quint64 m_uploadedSerial = 0;
};

} // namespace QuickUI::Components::Internal

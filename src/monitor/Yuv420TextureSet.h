#pragma once

#include "GlobalVideoRenderer.h"

#include <QSize>

#include <memory>

class QRhi;
class QRhiResourceUpdateBatch;
class QRhiTexture;

class Yuv420TextureSet
{
public:
    Yuv420TextureSet() = default;
    ~Yuv420TextureSet();

    bool ensureTextures(QRhi* rhi, const QSize& ySize, const QSize& uSize, const QSize& vSize);
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

private:
    struct TextureDeleter
    {
        void operator()(QRhiTexture* texture) const;
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
    void releasePlane(PlaneTexture& plane);

    QRhi* m_rhi = nullptr;
    PlaneTexture m_yPlane;
    PlaneTexture m_uPlane;
    PlaneTexture m_vPlane;
    quint64 m_uploadedSerial = 0;
};

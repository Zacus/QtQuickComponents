#include "Yuv420TextureSet.h"

#include <rhi/qrhi.h>

void Yuv420TextureSet::TextureDeleter::operator()(QRhiTexture* texture) const
{
    delete texture;
}

Yuv420TextureSet::PlaneTexture::~PlaneTexture() = default;

Yuv420TextureSet::~Yuv420TextureSet()
{
    release();
}

bool Yuv420TextureSet::ensureTextures(QRhi* rhi, const QSize& ySize, const QSize& uSize, const QSize& vSize)
{
    if (!rhi || !ySize.isValid() || !uSize.isValid() || !vSize.isValid()) {
        release();
        return false;
    }

    if (m_rhi != rhi) {
        release();
        m_rhi = rhi;
    }

    if (!ensurePlane(m_yPlane, ySize) || !ensurePlane(m_uPlane, uSize) || !ensurePlane(m_vPlane, vSize)) {
        release();
        return false;
    }

    return true;
}

void Yuv420TextureSet::release()
{
    releasePlane(m_yPlane);
    releasePlane(m_uPlane);
    releasePlane(m_vPlane);
    m_rhi = nullptr;
}

bool Yuv420TextureSet::isValid() const
{
    return m_rhi
        && m_yPlane.texture
        && m_uPlane.texture
        && m_vPlane.texture;
}

bool Yuv420TextureSet::ensurePlane(PlaneTexture& plane, const QSize& size)
{
    if (plane.texture && plane.size == size)
        return true;

    releasePlane(plane);

    std::unique_ptr<QRhiTexture, TextureDeleter> texture(m_rhi->newTexture(QRhiTexture::R8, size));
    if (!texture || !texture->create())
        return false;

    plane.size = size;
    plane.texture = std::move(texture);
    return true;
}

void Yuv420TextureSet::releasePlane(PlaneTexture& plane)
{
    if (plane.texture) {
        plane.texture->destroy();
        plane.texture.reset();
    }
    plane.size = QSize();
}

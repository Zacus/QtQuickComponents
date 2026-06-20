#include "Yuv420TextureSet.h"

#include <rhi/qrhi.h>

namespace {

int planeMinimumSize(int stride, int rows, int rowWidth)
{
    if (stride <= 0 || rows <= 0 || rowWidth <= 0)
        return 0;
    return stride * (rows - 1) + rowWidth;
}

bool hasPlaneBytes(const QByteArray& bytes, int stride, const QSize& size)
{
    return size.isValid()
        && stride >= size.width()
        && bytes.size() >= planeMinimumSize(stride, size.height(), size.width());
}

bool isValidYuv420Frame(const GlobalVideoRenderer::Yuv420Frame& frame)
{
    if (frame.width <= 0 || frame.height <= 0)
        return false;
    if ((frame.width % 2) != 0 || (frame.height % 2) != 0)
        return false;

    const QSize ySize(frame.width, frame.height);
    const QSize chromaSize(frame.width / 2, frame.height / 2);
    return hasPlaneBytes(frame.yPlane, frame.yStride, ySize)
        && hasPlaneBytes(frame.uPlane, frame.uStride, chromaSize)
        && hasPlaneBytes(frame.vPlane, frame.vStride, chromaSize);
}

} // namespace

void Yuv420TextureSet::TextureDeleter::operator()(QRhiTexture* texture) const
{
    delete texture;
}

void Yuv420TextureSet::SamplerDeleter::operator()(QRhiSampler* sampler) const
{
    delete sampler;
}

void Yuv420TextureSet::ShaderResourceBindingsDeleter::operator()(QRhiShaderResourceBindings* bindings) const
{
    delete bindings;
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

    const bool resourcesWillChange = m_rhi != rhi
        || !m_yPlane.texture
        || !m_uPlane.texture
        || !m_vPlane.texture
        || m_yPlane.size != ySize
        || m_uPlane.size != uSize
        || m_vPlane.size != vSize;

    if (m_rhi != rhi) {
        release();
        m_rhi = rhi;
    }

    if (resourcesWillChange)
        releaseShaderResourceBindings();

    if (!ensurePlane(m_yPlane, ySize) || !ensurePlane(m_uPlane, uSize) || !ensurePlane(m_vPlane, vSize)) {
        release();
        return false;
    }

    if (resourcesWillChange)
        m_uploadedSerial = 0;

    return true;
}

bool Yuv420TextureSet::ensureShaderResources(QRhi* rhi)
{
    if (!rhi || !isValid() || rhi != m_rhi)
        return false;

    return ensureSampler() && ensureShaderResourceBindings();
}

bool Yuv420TextureSet::uploadFrame(QRhi* rhi,
                                   QRhiResourceUpdateBatch* updates,
                                   const GlobalVideoRenderer::Yuv420Frame& frame,
                                   quint64 serial)
{
    if (!rhi || !updates || serial == 0 || !isValidYuv420Frame(frame))
        return false;

    if (isValid() && m_uploadedSerial == serial)
        return true;

    const QSize ySize(frame.width, frame.height);
    const QSize chromaSize(frame.width / 2, frame.height / 2);
    if (!ensureTextures(rhi, ySize, chromaSize, chromaSize))
        return false;

    if (!uploadPlane(updates, m_yPlane, frame.yPlane, frame.yStride, ySize)
        || !uploadPlane(updates, m_uPlane, frame.uPlane, frame.uStride, chromaSize)
        || !uploadPlane(updates, m_vPlane, frame.vPlane, frame.vStride, chromaSize)) {
        return false;
    }

    m_uploadedSerial = serial;
    return true;
}

void Yuv420TextureSet::release()
{
    releaseShaderResourceBindings();
    if (m_sampler) {
        m_sampler->destroy();
        m_sampler.reset();
    }
    releasePlane(m_yPlane);
    releasePlane(m_uPlane);
    releasePlane(m_vPlane);
    m_rhi = nullptr;
    m_uploadedSerial = 0;
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

bool Yuv420TextureSet::uploadPlane(QRhiResourceUpdateBatch* updates,
                                   const PlaneTexture& plane,
                                   const QByteArray& bytes,
                                   int stride,
                                   const QSize& sourceSize)
{
    if (!updates || !plane.texture || !hasPlaneBytes(bytes, stride, sourceSize))
        return false;

    QRhiTextureSubresourceUploadDescription subresource(bytes);
    subresource.setDataStride(quint32(stride));
    subresource.setSourceSize(sourceSize);

    updates->uploadTexture(plane.texture.get(), QRhiTextureUploadDescription(QRhiTextureUploadEntry(0, 0, subresource)));
    return true;
}

bool Yuv420TextureSet::ensureSampler()
{
    if (m_sampler)
        return true;

    std::unique_ptr<QRhiSampler, SamplerDeleter> sampler(
        m_rhi->newSampler(QRhiSampler::Linear,
                          QRhiSampler::Linear,
                          QRhiSampler::None,
                          QRhiSampler::ClampToEdge,
                          QRhiSampler::ClampToEdge));
    if (!sampler || !sampler->create())
        return false;

    m_sampler = std::move(sampler);
    return true;
}

bool Yuv420TextureSet::ensureShaderResourceBindings()
{
    if (m_shaderResourceBindings)
        return true;

    if (!m_sampler || !m_yPlane.texture || !m_uPlane.texture || !m_vPlane.texture)
        return false;

    std::unique_ptr<QRhiShaderResourceBindings, ShaderResourceBindingsDeleter> bindings(m_rhi->newShaderResourceBindings());
    if (!bindings)
        return false;

    bindings->setBindings({
        QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, m_yPlane.texture.get(), m_sampler.get()),
        QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, m_uPlane.texture.get(), m_sampler.get()),
        QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, m_vPlane.texture.get(), m_sampler.get()),
    });

    if (!bindings->create())
        return false;

    m_shaderResourceBindings = std::move(bindings);
    return true;
}

void Yuv420TextureSet::releaseShaderResourceBindings()
{
    if (m_shaderResourceBindings) {
        m_shaderResourceBindings->destroy();
        m_shaderResourceBindings.reset();
    }
}

void Yuv420TextureSet::releasePlane(PlaneTexture& plane)
{
    if (plane.texture) {
        plane.texture->destroy();
        plane.texture.reset();
    }
    plane.size = QSize();
}

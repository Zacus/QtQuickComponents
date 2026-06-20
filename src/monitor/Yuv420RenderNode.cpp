#include "Yuv420RenderNode.h"

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
}

void Yuv420RenderNode::releaseResources()
{
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

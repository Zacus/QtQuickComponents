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
}

void Yuv420RenderNode::setSnapshot(const GlobalVideoRenderer::Yuv420Snapshot& snapshot)
{
    m_snapshot = deepCopySnapshot(snapshot);
}

void Yuv420RenderNode::setRect(const QRectF& rect)
{
    m_rect = rect;
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

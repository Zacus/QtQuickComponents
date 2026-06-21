#include "VideoSurface.h"

#include "GlobalVideoRenderer.h"
#include "Yuv420RenderNode.h"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QSGTexture>

namespace {

QRectF fittedContentRect(const QRectF& bounds, const QSize& contentSize)
{
    if (bounds.width() <= 0.0
        || bounds.height() <= 0.0
        || contentSize.width() <= 0
        || contentSize.height() <= 0) {
        return QRectF();
    }

    const qreal contentAspect = qreal(contentSize.width()) / qreal(contentSize.height());
    const qreal boundsAspect = bounds.width() / bounds.height();

    QSizeF fittedSize;
    if (boundsAspect > contentAspect) {
        fittedSize.setHeight(bounds.height());
        fittedSize.setWidth(bounds.height() * contentAspect);
    } else {
        fittedSize.setWidth(bounds.width());
        fittedSize.setHeight(bounds.width() / contentAspect);
    }

    return QRectF(bounds.x() + (bounds.width() - fittedSize.width()) / 2.0,
                  bounds.y() + (bounds.height() - fittedSize.height()) / 2.0,
                  fittedSize.width(),
                  fittedSize.height());
}

QSize yuvFrameSize(const GlobalVideoRenderer::Yuv420Frame& frame)
{
    return QSize(frame.width, frame.height);
}

} // namespace

VideoSurface::VideoSurface(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

void VideoSurface::setChannelId(int channelId)
{
    if (m_channelId == channelId)
        return;

    m_channelId = channelId;
    emit channelIdChanged();
    refreshFrameState();
    update();
}

void VideoSurface::setVideoSink(QObject* videoSink)
{
    if (m_videoSink == videoSink)
        return;

    m_videoSink = videoSink;
    reconnectRenderer();
    refreshFrameState();
    emit videoSinkChanged();
    update();
}

QSGNode* VideoSurface::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    if (!m_renderer || m_channelId < 0) {
        delete oldNode;
        return nullptr;
    }

    if (m_activeFrameFormat == Yuv420Frame) {
        const auto snapshot = m_renderer->yuv420Snapshot(m_channelId);
        if (!snapshot.isValid()) {
            delete oldNode;
            return nullptr;
        }

        QRhi* rhi = window() ? window()->rhi() : nullptr;
        const QRectF videoRect = fittedContentRect(boundingRect(), yuvFrameSize(snapshot.frame));
        auto* node = dynamic_cast<Yuv420RenderNode*>(oldNode);
        if (!node) {
            delete oldNode;
            node = new Yuv420RenderNode(snapshot, videoRect);
        } else {
            node->setSnapshot(snapshot);
            node->setRect(videoRect);
        }
        node->setRhi(rhi);
        return node;
    }

    if (!window()) {
        delete oldNode;
        return nullptr;
    }

    auto* node = dynamic_cast<QSGSimpleTextureNode*>(oldNode);
    if (!node)
        delete oldNode;

    const auto snapshot = m_renderer->frameSnapshot(m_channelId);
    if (!snapshot.isValid()) {
        delete node;
        return nullptr;
    }

    if (!node)
        node = new QSGSimpleTextureNode;

    QSGTexture* texture = window()->createTextureFromImage(snapshot.image);
    node->setTexture(texture);
    node->setOwnsTexture(true);
    node->setRect(fittedContentRect(boundingRect(), snapshot.image.size()));

    return node;
}

void VideoSurface::reconnectRenderer()
{
    if (m_frameReadyConnection)
        QObject::disconnect(m_frameReadyConnection);
    if (m_channelClearedConnection)
        QObject::disconnect(m_channelClearedConnection);
    if (m_clearedConnection)
        QObject::disconnect(m_clearedConnection);

    m_renderer = qobject_cast<GlobalVideoRenderer*>(m_videoSink.data());
    if (!m_renderer)
        return;

    m_frameReadyConnection = connect(m_renderer, &GlobalVideoRenderer::frameReady,
                                     this, [this](int channelId, quint64 serial) {
        if (channelId != m_channelId)
            return;
        setFrameState(true, serial, frameFormatForCurrentChannel());
        update();
    });

    m_channelClearedConnection = connect(m_renderer, &GlobalVideoRenderer::channelCleared,
                                         this, [this](int channelId) {
        if (channelId != m_channelId)
            return;
        setFrameState(false, 0, NoFrame);
        update();
    });

    m_clearedConnection = connect(m_renderer, &GlobalVideoRenderer::cleared,
                                  this, [this]() {
        setFrameState(false, 0, NoFrame);
        update();
    });
}

void VideoSurface::refreshFrameState()
{
    if (!m_renderer || m_channelId < 0) {
        setFrameState(false, 0, NoFrame);
        return;
    }

    const quint64 serial = m_renderer->frameSerial(m_channelId);
    setFrameState(serial != 0, serial, serial != 0 ? frameFormatForCurrentChannel() : NoFrame);
}

VideoSurface::FrameFormat VideoSurface::frameFormatForCurrentChannel() const
{
    if (!m_renderer || m_channelId < 0)
        return NoFrame;

    return m_renderer->hasYuv420Frame(m_channelId) ? Yuv420Frame : RgbaFrame;
}

void VideoSurface::setFrameState(bool hasFrame, quint64 serial, VideoSurface::FrameFormat frameFormat)
{
    if (!hasFrame)
        frameFormat = NoFrame;

    if (m_hasFrame == hasFrame
        && m_currentSerial == serial
        && m_activeFrameFormat == frameFormat) {
        return;
    }

    m_hasFrame = hasFrame;
    m_currentSerial = serial;
    m_activeFrameFormat = frameFormat;
    emit frameStateChanged();
}

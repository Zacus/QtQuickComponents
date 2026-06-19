#include "VideoSurface.h"

#include "GlobalVideoRenderer.h"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QSGTexture>

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
    auto* node = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!m_renderer || m_channelId < 0 || !window()) {
        delete node;
        return nullptr;
    }

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
    node->setRect(boundingRect());

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

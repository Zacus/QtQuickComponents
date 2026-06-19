#pragma once

#include <QPointer>
#include <QQuickItem>
#include <QQmlEngine>

class GlobalVideoRenderer;
class QSGNode;

class VideoSurface : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int channelId READ channelId WRITE setChannelId NOTIFY channelIdChanged)
    Q_PROPERTY(QObject* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(bool hasFrame READ hasFrame NOTIFY frameStateChanged)
    Q_PROPERTY(quint64 currentSerial READ currentSerial NOTIFY frameStateChanged)
    Q_PROPERTY(FrameFormat activeFrameFormat READ activeFrameFormat NOTIFY frameStateChanged)

public:
    enum FrameFormat {
        NoFrame = 0,
        RgbaFrame,
        Yuv420Frame
    };
    Q_ENUM(FrameFormat)

    explicit VideoSurface(QQuickItem* parent = nullptr);

    int channelId() const { return m_channelId; }
    QObject* videoSink() const { return m_videoSink.data(); }
    bool hasFrame() const { return m_hasFrame; }
    quint64 currentSerial() const { return m_currentSerial; }
    FrameFormat activeFrameFormat() const { return m_activeFrameFormat; }

    void setChannelId(int channelId);
    void setVideoSink(QObject* videoSink);

    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

signals:
    void channelIdChanged();
    void videoSinkChanged();
    void frameStateChanged();

private:
    void reconnectRenderer();
    void refreshFrameState();
    FrameFormat frameFormatForCurrentChannel() const;
    void setFrameState(bool hasFrame, quint64 serial, FrameFormat frameFormat);

    int m_channelId = -1;
    QPointer<QObject> m_videoSink;
    QPointer<GlobalVideoRenderer> m_renderer;
    QMetaObject::Connection m_frameReadyConnection;
    QMetaObject::Connection m_channelClearedConnection;
    QMetaObject::Connection m_clearedConnection;
    bool m_hasFrame = false;
    quint64 m_currentSerial = 0;
    FrameFormat m_activeFrameFormat = NoFrame;
};

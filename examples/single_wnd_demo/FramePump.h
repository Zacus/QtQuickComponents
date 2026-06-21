#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QTimer>
#include <QVariantList>

class FramePump : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QObject* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(QVariantList channels READ channels WRITE setChannels NOTIFY channelsChanged)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    Q_PROPERTY(int frameWidth READ frameWidth WRITE setFrameWidth NOTIFY frameSizeChanged)
    Q_PROPERTY(int frameHeight READ frameHeight WRITE setFrameHeight NOTIFY frameSizeChanged)

public:
    explicit FramePump(QObject* parent = nullptr);

    QObject* videoSink() const { return m_videoSink.data(); }
    void setVideoSink(QObject* videoSink);

    QVariantList channels() const { return m_channels; }
    void setChannels(const QVariantList& channels);

    bool running() const { return m_timer.isActive(); }
    void setRunning(bool running);

    int interval() const { return m_timer.interval(); }
    void setInterval(int interval);

    int frameWidth() const { return m_frameWidth; }
    void setFrameWidth(int frameWidth);

    int frameHeight() const { return m_frameHeight; }
    void setFrameHeight(int frameHeight);

signals:
    void videoSinkChanged();
    void channelsChanged();
    void runningChanged();
    void intervalChanged();
    void frameSizeChanged();
    void framePushed(int channelId, quint64 serial);

private:
    void pushFrames();
    void updateTimerState();

    QPointer<QObject> m_videoSink;
    QTimer m_timer;
    QVariantList m_channels;
    int m_frameWidth = 320;
    int m_frameHeight = 180;
    quint64 m_tick = 0;
    bool m_requestedRunning = false;
};

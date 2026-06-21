#pragma once

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QQmlEngine>
#include <QByteArray>

namespace QuickUI::Components::Internal {

class GlobalVideoRenderer : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GlobalVideoRenderer)

public:
    struct FrameSnapshot
    {
        QImage image;
        quint64 serial = 0;

        bool isValid() const { return !image.isNull() && serial != 0; }
    };

    struct Yuv420Frame
    {
        int width = 0;
        int height = 0;
        int yStride = 0;
        int uStride = 0;
        int vStride = 0;
        QByteArray yPlane;
        QByteArray uPlane;
        QByteArray vPlane;
    };

    struct Yuv420Snapshot
    {
        Yuv420Frame frame;
        quint64 serial = 0;

        bool isValid() const { return frame.width > 0 && frame.height > 0 && serial != 0; }
    };

    explicit GlobalVideoRenderer(QObject* parent = nullptr);

    bool pushFrame(int channelId, const QImage& image);
    bool pushYuv420Frame(int channelId, const Yuv420Frame& frame);
    Q_INVOKABLE void clearChannel(int channelId);
    Q_INVOKABLE void clear();

    FrameSnapshot frameSnapshot(int channelId) const;
    Yuv420Snapshot yuv420Snapshot(int channelId) const;
    Q_INVOKABLE bool hasFrame(int channelId) const;
    Q_INVOKABLE bool hasYuv420Frame(int channelId) const;
    Q_INVOKABLE quint64 frameSerial(int channelId) const;

signals:
    void frameReady(int channelId, quint64 serial);
    void channelCleared(int channelId);
    void cleared();

private:
    bool storeFrameSnapshot(int channelId, FrameSnapshot snapshot, const Yuv420Snapshot* yuvSnapshot);

    mutable QMutex m_mutex;
    QHash<int, FrameSnapshot> m_frames;
    QHash<int, Yuv420Snapshot> m_yuvFrames;
    quint64 m_nextSerial = 1;
};

} // namespace QuickUI::Components::Internal

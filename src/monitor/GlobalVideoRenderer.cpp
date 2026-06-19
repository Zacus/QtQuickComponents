#include "GlobalVideoRenderer.h"

#include <QMutexLocker>
#include <algorithm>

namespace {

int clampByte(int value)
{
    return std::clamp(value, 0, 255);
}

int planeMinimumSize(int stride, int rows, int rowWidth)
{
    if (stride <= 0 || rows <= 0 || rowWidth <= 0)
        return 0;
    return stride * (rows - 1) + rowWidth;
}

bool isValidYuv420Frame(const GlobalVideoRenderer::Yuv420Frame& frame)
{
    if (frame.width <= 0 || frame.height <= 0)
        return false;
    if ((frame.width % 2) != 0 || (frame.height % 2) != 0)
        return false;
    if (frame.yStride < frame.width)
        return false;
    if (frame.uStride < frame.width / 2 || frame.vStride < frame.width / 2)
        return false;

    const int chromaRows = frame.height / 2;
    const int chromaWidth = frame.width / 2;
    return frame.yPlane.size() >= planeMinimumSize(frame.yStride, frame.height, frame.width)
        && frame.uPlane.size() >= planeMinimumSize(frame.uStride, chromaRows, chromaWidth)
        && frame.vPlane.size() >= planeMinimumSize(frame.vStride, chromaRows, chromaWidth);
}

QByteArray deepCopyBytes(const QByteArray& data)
{
    return QByteArray(data.constData(), data.size());
}

GlobalVideoRenderer::Yuv420Frame deepCopyYuv420Frame(const GlobalVideoRenderer::Yuv420Frame& frame)
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

int samplePlane(const QByteArray& plane, int stride, int x, int y)
{
    return static_cast<unsigned char>(plane.at(y * stride + x));
}

QRgb convertYuvToRgba(int y, int u, int v)
{
    const int c = y - 16;
    const int d = u - 128;
    const int e = v - 128;

    const int r = clampByte((298 * c + 409 * e + 128) >> 8);
    const int g = clampByte((298 * c - 100 * d - 208 * e + 128) >> 8);
    const int b = clampByte((298 * c + 516 * d + 128) >> 8);
    return qRgba(r, g, b, 255);
}

QImage convertYuv420ToImage(const GlobalVideoRenderer::Yuv420Frame& frame)
{
    QImage image(frame.width, frame.height, QImage::Format_RGBA8888);
    for (int y = 0; y < frame.height; ++y) {
        auto* scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < frame.width; ++x) {
            const int yy = samplePlane(frame.yPlane, frame.yStride, x, y);
            const int uu = samplePlane(frame.uPlane, frame.uStride, x / 2, y / 2);
            const int vv = samplePlane(frame.vPlane, frame.vStride, x / 2, y / 2);
            scanLine[x] = convertYuvToRgba(yy, uu, vv);
        }
    }
    return image;
}

} // namespace

GlobalVideoRenderer::GlobalVideoRenderer(QObject* parent)
    : QObject(parent)
{
}

bool GlobalVideoRenderer::pushFrame(int channelId, const QImage& image)
{
    if (channelId < 0 || image.isNull())
        return false;

    FrameSnapshot snapshot;
    snapshot.image = image.convertToFormat(QImage::Format_RGBA8888).copy();
    return storeFrameSnapshot(channelId, snapshot, nullptr);
}

bool GlobalVideoRenderer::pushYuv420Frame(int channelId, const Yuv420Frame& frame)
{
    if (channelId < 0 || !isValidYuv420Frame(frame))
        return false;

    FrameSnapshot rgbaSnapshot;
    rgbaSnapshot.image = convertYuv420ToImage(frame);

    Yuv420Snapshot yuvSnapshot;
    yuvSnapshot.frame = deepCopyYuv420Frame(frame);
    return storeFrameSnapshot(channelId, rgbaSnapshot, &yuvSnapshot);
}

void GlobalVideoRenderer::clearChannel(int channelId)
{
    bool removed = false;
    {
        QMutexLocker locker(&m_mutex);
        removed = m_frames.remove(channelId) > 0;
        removed = m_yuvFrames.remove(channelId) > 0 || removed;
    }

    if (removed)
        emit channelCleared(channelId);
}

void GlobalVideoRenderer::clear()
{
    bool hadFrames = false;
    {
        QMutexLocker locker(&m_mutex);
        hadFrames = !m_frames.isEmpty();
        m_frames.clear();
        hadFrames = !m_yuvFrames.isEmpty() || hadFrames;
        m_yuvFrames.clear();
    }

    if (hadFrames)
        emit cleared();
}

GlobalVideoRenderer::FrameSnapshot GlobalVideoRenderer::frameSnapshot(int channelId) const
{
    QMutexLocker locker(&m_mutex);
    return m_frames.value(channelId);
}

GlobalVideoRenderer::Yuv420Snapshot GlobalVideoRenderer::yuv420Snapshot(int channelId) const
{
    QMutexLocker locker(&m_mutex);
    return m_yuvFrames.value(channelId);
}

bool GlobalVideoRenderer::hasFrame(int channelId) const
{
    QMutexLocker locker(&m_mutex);
    return m_frames.contains(channelId);
}

bool GlobalVideoRenderer::hasYuv420Frame(int channelId) const
{
    QMutexLocker locker(&m_mutex);
    return m_yuvFrames.contains(channelId);
}

quint64 GlobalVideoRenderer::frameSerial(int channelId) const
{
    QMutexLocker locker(&m_mutex);
    return m_frames.value(channelId).serial;
}

bool GlobalVideoRenderer::storeFrameSnapshot(int channelId, FrameSnapshot snapshot, const Yuv420Snapshot* yuvSnapshot)
{
    {
        QMutexLocker locker(&m_mutex);
        snapshot.serial = m_nextSerial++;
        m_frames.insert(channelId, snapshot);

        if (yuvSnapshot) {
            Yuv420Snapshot storedYuv = *yuvSnapshot;
            storedYuv.serial = snapshot.serial;
            m_yuvFrames.insert(channelId, storedYuv);
        } else {
            m_yuvFrames.remove(channelId);
        }
    }

    emit frameReady(channelId, snapshot.serial);
    return true;
}

#include "FramePump.h"

#include "GlobalVideoRenderer.h"

#include <QColor>
#include <QImage>
#include <QtGlobal>

namespace {

QImage makeFrame(int width, int height, int channelIndex, quint64 tick)
{
    QImage image(width, height, QImage::Format_RGBA8888);
    const int hue = (channelIndex * 57 + int(tick * 9)) % 360;
    image.fill(QColor::fromHsv(hue, 180, 190));

    const int bandWidth = qMax(1, width / 10);
    const int xOffset = int((tick * 7 + quint64(channelIndex * bandWidth)) % quint64(width));
    const QColor bandColor = QColor::fromHsv((hue + 32) % 360, 210, 235);
    for (int y = 0; y < height; ++y) {
        auto* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < bandWidth; ++x)
            line[(xOffset + x) % width] = bandColor.rgba();
    }

    return image;
}

} // namespace

FramePump::FramePump(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(120);
    connect(&m_timer, &QTimer::timeout, this, &FramePump::pushFrames);
}

void FramePump::setVideoSink(QObject* videoSink)
{
    if (m_videoSink == videoSink)
        return;

    m_videoSink = videoSink;
    emit videoSinkChanged();
    updateTimerState();
}

void FramePump::setChannels(const QVariantList& channels)
{
    if (m_channels == channels)
        return;

    m_channels = channels;
    emit channelsChanged();
    updateTimerState();
}

void FramePump::setRunning(bool running)
{
    if (m_requestedRunning == running)
        return;

    m_requestedRunning = running;
    emit runningChanged();
    updateTimerState();
}

void FramePump::setInterval(int interval)
{
    const int boundedInterval = qMax(16, interval);
    if (m_timer.interval() == boundedInterval)
        return;

    m_timer.setInterval(boundedInterval);
    emit intervalChanged();
}

void FramePump::setFrameWidth(int frameWidth)
{
    const int boundedWidth = qMax(2, frameWidth);
    if (m_frameWidth == boundedWidth)
        return;

    m_frameWidth = boundedWidth;
    emit frameSizeChanged();
}

void FramePump::setFrameHeight(int frameHeight)
{
    const int boundedHeight = qMax(2, frameHeight);
    if (m_frameHeight == boundedHeight)
        return;

    m_frameHeight = boundedHeight;
    emit frameSizeChanged();
}

void FramePump::pushFrames()
{
    auto* renderer = qobject_cast<GlobalVideoRenderer*>(m_videoSink.data());
    if (!renderer)
        return;

    ++m_tick;
    for (int index = 0; index < m_channels.size(); ++index) {
        bool ok = false;
        const int channelId = m_channels.at(index).toInt(&ok);
        if (!ok || channelId < 0)
            continue;

        const QImage frame = makeFrame(m_frameWidth, m_frameHeight, index, m_tick);
        if (!renderer->pushFrame(channelId, frame))
            continue;

        emit framePushed(channelId, renderer->frameSerial(channelId));
    }
}

void FramePump::updateTimerState()
{
    const bool canRun = m_requestedRunning
        && !m_videoSink.isNull()
        && !m_channels.isEmpty();

    if (canRun && !m_timer.isActive()) {
        m_timer.start();
        pushFrames();
    } else if (!canRun && m_timer.isActive()) {
        m_timer.stop();
    }
}

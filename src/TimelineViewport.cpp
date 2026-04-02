#include "TimelineViewport.h"

#include <QtCore/qdebug.h>
#include <algorithm>

// ── 构造 ──────────────────────────────────────────────────────────────

TimelineViewport::TimelineViewport(QObject* parent)
    : QObject(parent)
{
    // 构造时不预设视口，等待 totalStart/totalEnd 设置后由 fitAll() 决定初始视图。
    // m_viewStart = m_viewEnd = 0 表示"未初始化"，recalcPixelsPerMs 会返回 0。
    recalcPixelsPerMs();
}

// ── setters ───────────────────────────────────────────────────────────

void TimelineViewport::setViewStart(qint64 v)
{
    // 单独设置 viewStart：保持 viewSpan 不变，整体平移
    const qint64 span = m_viewEnd - m_viewStart;
    if (applyView(v, v + span))
        emit viewChanged();
}

void TimelineViewport::setViewEnd(qint64 v)
{
    const qint64 span = m_viewEnd - m_viewStart;
    if (applyView(v - span, v))
        emit viewChanged();
}

void TimelineViewport::setViewWidth(qreal v)
{
    const qreal clamped = qMax(1.0, v);
    if (qFuzzyCompare(m_viewWidth, clamped)) return;
    m_viewWidth = clamped;
    recalcPixelsPerMs();
    emit viewWidthChanged();
    emit viewChanged();   // pixelsPerMs 也变了
}

void TimelineViewport::setTotalStart(qint64 v)
{
    if (m_totalStart == v) return;
    m_totalStart = v;
    emit totalRangeChanged();
}

void TimelineViewport::setTotalEnd(qint64 v)
{
    if (m_totalEnd == v) return;
    m_totalEnd = v;
    emit totalRangeChanged();
}

void TimelineViewport::setMinZoomFactor(qreal v)
{
    if (qFuzzyCompare(m_minZoomFactor, v)) return;
    m_minZoomFactor = v;
    emit limitsChanged();
}

void TimelineViewport::setMaxZoomFactor(qreal v)
{
    if (qFuzzyCompare(m_maxZoomFactor, v)) return;
    m_maxZoomFactor = v;
    emit limitsChanged();
}

// ── 视口操作 ──────────────────────────────────────────────────────────

void TimelineViewport::zoomAt(qreal anchorPx, qreal factor)
{
    if (factor <= 0.0 || m_pixelsPerMs <= 0.0) return;

    // 锚点对应的时间（缩放后此时刻在屏幕上的位置不变）
    const qreal anchorTime = static_cast<qreal>(m_viewStart)
                             + anchorPx / m_pixelsPerMs;

    // 目标 span
    qreal newSpanF = static_cast<qreal>(m_viewEnd - m_viewStart) / factor;

    // span 下限：kMinViewSpan（放大极限）
    const qreal minSpan = static_cast<qreal>(kMinViewSpan);

    // span 上限：数据总范围（缩小不超过数据跨度，保证数据始终铺满屏幕）。
    // 若数据范围超过 kMaxViewSpan（如超5年），则以 kMaxViewSpan 为上限，
    // 此时用 fitAll() 查看全貌。
    const qreal dataRange = static_cast<qreal>(m_totalEnd - m_totalStart);
    const qreal maxSpan   = (dataRange > 0.0)
                            ? qMin(static_cast<qreal>(kMaxViewSpan), dataRange)
                            : static_cast<qreal>(kMaxViewSpan);

    if (newSpanF < minSpan) newSpanF = minSpan;
    if (newSpanF > maxSpan) newSpanF = maxSpan;

    // 以锚点时间为中心重新推算 viewStart：
    // anchorPx = (anchorTime - newStart) * viewWidth / newSpan
    // => newStart = anchorTime - anchorPx * newSpan / viewWidth
    const qreal newStartF = anchorTime
                            - anchorPx * newSpanF / static_cast<qreal>(m_viewWidth);
    const qint64 newStart = qRound64(newStartF);
    const qint64 newEnd   = qRound64(newStartF + newSpanF);

    if (applyView(newStart, newEnd))
        emit viewChanged();
}

void TimelineViewport::panBy(qreal deltaPixels)
{
    if (qFuzzyIsNull(deltaPixels) || m_pixelsPerMs <= 0.0) return;

    // 正 deltaPixels：向左拖拽，时间向未来方向移动（viewStart 增大）
    const qint64 deltaMs = static_cast<qint64>(deltaPixels / m_pixelsPerMs);
    if (deltaMs == 0) return;

    const qint64 span = m_viewEnd - m_viewStart;
    if (applyView(m_viewStart + deltaMs, m_viewStart + deltaMs + span))
        emit viewChanged();
}

void TimelineViewport::scrollToTime(qint64 timeMs)
{
    const qint64 span = m_viewEnd - m_viewStart;
    if (applyView(timeMs, timeMs + span))
        emit viewChanged();
}

void TimelineViewport::centerOnTime(qint64 timeMs)
{
    const qint64 halfSpan = (m_viewEnd - m_viewStart) / 2;
    if (applyView(timeMs - halfSpan, timeMs + halfSpan))
        emit viewChanged();
}

void TimelineViewport::fitRange(qint64 startMs, qint64 endMs)
{
    if (startMs >= endMs) return;

    // 加 5% 边距
    const qint64 margin = (endMs - startMs) / 20;
    qint64 start = startMs - margin;
    qint64 end   = endMs   + margin;

    // fitRange 是用户/初始化主动操作，不受 kMaxViewSpan 限制，
    // 但 span 需要至少 kMinViewSpan
    if (end - start < kMinViewSpan) {
        const qint64 center = start + (end - start) / 2;
        start = center - kMinViewSpan / 2;
        end   = start + kMinViewSpan;
    }

    clampToBounds(start, end);

    if (start == m_viewStart && end == m_viewEnd) return;
    m_viewStart = start;
    m_viewEnd   = end;
    recalcPixelsPerMs();
    emit viewChanged();
}

void TimelineViewport::fitAll()
{
    if (m_totalStart >= m_totalEnd) return;
    fitRange(m_totalStart, m_totalEnd);
}

// ── 私有工具 ──────────────────────────────────────────────────────────

void TimelineViewport::recalcPixelsPerMs()
{
    const qint64 span = m_viewEnd - m_viewStart;
    if (span <= 0 || m_viewWidth <= 0.0) {
        m_pixelsPerMs = 0.0;
        return;
    }
    m_pixelsPerMs = m_viewWidth / static_cast<qreal>(span);
}

bool TimelineViewport::applyView(qint64 start, qint64 end)
{
    const qint64 span = end - start;

    // 拒绝无效 span（负数或零）
    // span 的合理上下限由各调用方（zoomAt/fitRange/panBy）在传入前保证
    if (span <= 0)
        return false;

    clampToBounds(start, end);

    if (start == m_viewStart && end == m_viewEnd)
        return false;

    m_viewStart = start;
    m_viewEnd   = end;
    recalcPixelsPerMs();
    return true;
}

void TimelineViewport::clampToBounds(qint64& start, qint64& end) const
{
    if (m_totalStart >= m_totalEnd) return;

    const qint64 span      = end - start;
    const qint64 dataRange = m_totalEnd - m_totalStart;

    // margin 随 span 接近 dataRange 而收缩，但保留最小值 minMargin。
    // minMargin 保证即使缩到极限，用户仍可左右拖拽/键盘平移看到边缘区域。
    // 设为 dataRange 的 5%，不会让数据挤出太多，但足够平移响应。
    const qint64 minMargin = dataRange / 20;

    const qreal ratio  = (dataRange > 0)
                         ? qBound(0.0, 1.0 - static_cast<qreal>(span)
                                            / static_cast<qreal>(dataRange), 1.0)
                         : 0.0;
    const qint64 margin = qMax(minMargin,
                               static_cast<qint64>(ratio * static_cast<qreal>(span) * 0.5));

    const qint64 minStart = m_totalStart - margin;
    const qint64 maxEnd   = m_totalEnd   + margin;

    if (start < minStart) {
        start = minStart;
        end   = start + span;
    }
    if (end > maxEnd) {
        end   = maxEnd;
        start = end - span;
    }
}

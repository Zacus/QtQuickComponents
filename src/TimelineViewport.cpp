#include "TimelineViewport.h"

#include <QtCore/qdebug.h>
#include <algorithm>

// ── 构造 ──────────────────────────────────────────────────────────────

TimelineViewport::TimelineViewport(QObject* parent)
    : QObject(parent)
{
    // 初始视口：0 ~ 24h，等宽度加载后 fitAll() 会覆盖
    const qint64 oneDayMs = 24LL * 3600 * 1000;
    m_viewStart   = 0;
    m_viewEnd     = oneDayMs;
    m_totalStart  = 0;
    m_totalEnd    = oneDayMs;
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
    // 如果当前视口超出新范围，夹一次
    qint64 s = m_viewStart, e = m_viewEnd;
    clampToBounds(s, e);
    if (applyView(s, e)) emit viewChanged();
}

void TimelineViewport::setTotalEnd(qint64 v)
{
    if (m_totalEnd == v) return;
    m_totalEnd = v;
    emit totalRangeChanged();
    qint64 s = m_viewStart, e = m_viewEnd;
    clampToBounds(s, e);
    if (applyView(s, e)) emit viewChanged();
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

    // 锚点时间（缩放后此时刻在屏幕上的位置不变）
    const qreal anchorTime = static_cast<qreal>(m_viewStart)
                             + anchorPx / m_pixelsPerMs;

    // 新的每毫秒像素数
    const qreal newPpm = m_pixelsPerMs * factor;

    // 由新 ppm 反推新的 viewStart/viewEnd
    // anchorPx = (anchorTime - newViewStart) * newPpm
    // => newViewStart = anchorTime - anchorPx / newPpm
    const qreal newStartF = anchorTime - anchorPx / newPpm;
    const qreal newSpanF  = static_cast<qreal>(m_viewWidth) / newPpm;

    const qint64 newStart = static_cast<qint64>(qRound(newStartF));
    const qint64 newEnd   = static_cast<qint64>(qRound(newStartF + newSpanF));

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
    // 加 5% 边距，让两侧区间不紧贴屏幕边缘
    const qint64 margin = (endMs - startMs) / 20;
    if (applyView(startMs - margin, endMs + margin))
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
    // 1. span 限幅
    qint64 span = end - start;
    if (span < kMinViewSpan) {
        // 保持中心，扩展到最小 span
        const qint64 center = start + span / 2;
        start = center - kMinViewSpan / 2;
        end   = start + kMinViewSpan;
    } else if (span > kMaxViewSpan) {
        const qint64 center = start + span / 2;
        start = center - kMaxViewSpan / 2;
        end   = start + kMaxViewSpan;
    }

    // 2. 边界夹（仅当 totalStart < totalEnd 时才约束）
    clampToBounds(start, end);

    // 3. 检查是否真的变化了
    if (start == m_viewStart && end == m_viewEnd) return false;

    m_viewStart = start;
    m_viewEnd   = end;
    recalcPixelsPerMs();
    return true;
}

void TimelineViewport::clampToBounds(qint64& start, qint64& end) const
{
    // 只有在数据范围有效时才约束
    if (m_totalStart >= m_totalEnd) return;

    const qint64 span = end - start;

    // 不允许视口完全超出数据范围之外（保留至少 1 像素的可见数据）
    // 左边界：viewEnd > totalStart
    if (end <= m_totalStart) {
        end   = m_totalStart + 1;
        start = end - span;
    }
    // 右边界：viewStart < totalEnd
    if (start >= m_totalEnd) {
        start = m_totalEnd - 1;
        end   = start + span;
    }
}

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

    const qreal anchorTime = static_cast<qreal>(m_viewStart)
                             + anchorPx / m_pixelsPerMs;
    const qreal newPpm    = m_pixelsPerMs * factor;
    const qreal newStartF = anchorTime - anchorPx / newPpm;
    const qreal newSpanF  = static_cast<qreal>(m_viewWidth) / newPpm;

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
    qint64 span = end - start;

    if (span < kMinViewSpan) {
        const qint64 center = start + span / 2;
        start = center - kMinViewSpan / 2;
        end   = start + kMinViewSpan;
    } else if (span > kMaxViewSpan) {
        const qint64 center = start + span / 2;
        start = center - kMaxViewSpan / 2;
        end   = start + kMaxViewSpan;
    }

    clampToBounds(start, end);

    if (start == m_viewStart && end == m_viewEnd) {
        return false;
    }

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

    // 只阻止视口与数据范围完全分离（即两者必须有交集）。
    // 不强制把视口锁在 [totalStart, totalEnd] 内——缩放/平移时视口两端
    // 超出数据范围是正常的（边缘留白），强制夹紧会破坏锚点缩放的数学关系。

    // 视口完全跑到数据左侧：viewEnd <= totalStart → 把 viewEnd 拉回到 totalStart
    if (end <= m_totalStart) {
        end   = m_totalStart;
        start = end - span;
    }
    // 视口完全跑到数据右侧：viewStart >= totalEnd → 把 viewStart 拉回到 totalEnd
    else if (start >= m_totalEnd) {
        start = m_totalEnd;
        end   = start + span;
    }
    // 其余情况（有交集）：不做任何修改，允许视口两端超出数据范围
}

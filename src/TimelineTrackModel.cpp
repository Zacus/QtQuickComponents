#include "TimelineTrackModel.h"

#include <algorithm>

TimelineTrackModel::TimelineTrackModel(QObject* parent)
    : QObject(parent)
{}

// ── setters ───────────────────────────────────────────────────────────

void TimelineTrackModel::setModel(TimelineModel* v)
{
    if (m_model == v) return;

    if (m_model) {
        disconnect(m_model, &TimelineModel::countChanged,
                   this, &TimelineTrackModel::onDataChanged);
        disconnect(m_model, &TimelineModel::boundsChanged,
                   this, &TimelineTrackModel::onDataChanged);
    }

    m_model = v;

    if (m_model) {
        connect(m_model, &TimelineModel::countChanged,
                this, &TimelineTrackModel::onDataChanged);
        connect(m_model, &TimelineModel::boundsChanged,
                this, &TimelineTrackModel::onDataChanged);
    }

    emit modelChanged();
    rebuild();
}

void TimelineTrackModel::setViewport(TimelineViewport* v)
{
    if (m_viewport == v) return;

    if (m_viewport)
        disconnect(m_viewport, &TimelineViewport::viewChanged,
                   this, &TimelineTrackModel::onViewChanged);

    m_viewport = v;

    if (m_viewport)
        connect(m_viewport, &TimelineViewport::viewChanged,
                this, &TimelineTrackModel::onViewChanged);

    emit viewportChanged();
    rebuild();
}

void TimelineTrackModel::setMergeGapPx(qreal v)
{
    if (qFuzzyCompare(m_mergeGapPx, v)) return;
    m_mergeGapPx = v;
    emit mergeGapPxChanged();
    rebuild();
}

void TimelineTrackModel::setTrackHeight(qreal v)
{
    if (qFuzzyCompare(m_trackHeight, v)) return;
    m_trackHeight = v;
    emit trackHeightChanged();
    rebuild();
}

void TimelineTrackModel::setTrackY(qreal v)
{
    if (qFuzzyCompare(m_trackY, v)) return;
    m_trackY = v;
    emit trackYChanged();
    rebuild();
}

// ── 查询 ──────────────────────────────────────────────────────────────

int TimelineTrackModel::rectCount() const
{
    int total = 0;
    for (const auto& list : m_rects)
        total += list.size();
    return total;
}

QVariantList TimelineTrackModel::rectsForType(int type) const
{
    const QList<QRectF>& list = m_rects.value(type);
    QVariantList result;
    result.reserve(list.size());
    for (const QRectF& r : list)
        result.append(QVariant::fromValue(r));
    return result;
}

QVariantList TimelineTrackModel::allRects() const
{
    QVariantList result;
    for (auto it = m_rects.cbegin(); it != m_rects.cend(); ++it) {
        const int type = it.key();
        for (const QRectF& r : it.value()) {
            QVariantMap entry;
            entry[QStringLiteral("type")]   = type;
            entry[QStringLiteral("x")]      = r.x();
            entry[QStringLiteral("y")]      = r.y();
            entry[QStringLiteral("width")]  = r.width();
            entry[QStringLiteral("height")] = r.height();
            result.append(entry);
        }
    }
    return result;
}

// ── 槽 ────────────────────────────────────────────────────────────────

void TimelineTrackModel::onViewChanged() { rebuild(); }
void TimelineTrackModel::onDataChanged() { rebuild(); }

// ── 核心：预计算合并矩形 ──────────────────────────────────────────────

void TimelineTrackModel::rebuild()
{
    if (!m_model || !m_viewport) {
        if (!m_rects.isEmpty()) {
            m_rects.clear();
            emit rectsChanged();
        }
        return;
    }

    const qreal pixelsPerMs = m_viewport->pixelsPerMs();
    if (pixelsPerMs <= 0.0) return;

    const qint64 viewStart = m_viewport->viewStart();
    const qint64 viewEnd   = m_viewport->viewEnd();

    // 取视口内的原始区间（O(log N + K)，已在 C++ 里做了二分）
    // 注意：这里直接用 C++ 接口，不走 QVariantList，避免装箱开销
    const QList<TimelineSegment> segs = m_model->segmentsInRawRange(viewStart, viewEnd);

    // 按 type 分组，每组内部按 startMs 排序（TimelineModel 已保证升序）
    QHash<int, QList<TimelineSegment>> byType;
    for (const TimelineSegment& seg : segs)
        byType[seg.type()].append(seg);

    QHash<int, QList<QRectF>> newRects;

    for (auto it = byType.begin(); it != byType.end(); ++it) {
        const int type = it.key();
        const QList<TimelineSegment>& group = it.value();
        QList<QRectF>& rects = newRects[type];

        // 扫描合并：相邻矩形像素间距 < mergeGapPx 时合并
        for (const TimelineSegment& seg : group) {
            // 时间坐标 → 像素坐标，裁剪到视口边界
            qreal x1 = static_cast<qreal>(seg.startMs() - viewStart) * pixelsPerMs;
            qreal x2 = static_cast<qreal>(seg.endMs()   - viewStart) * pixelsPerMs;

            // 裁剪到 [0, viewWidth]
            const qreal vw = m_viewport->viewWidth();
            x1 = qBound(0.0, x1, vw);
            x2 = qBound(0.0, x2, vw);

            const qreal w = x2 - x1;
            if (w < 0.5) continue;  // 亚像素区间跳过

            if (!rects.isEmpty()) {
                QRectF& last = rects.last();
                // 与上一个矩形的间距 < mergeGapPx → 合并（扩展右边界）
                if (x1 - last.right() < m_mergeGapPx) {
                    last.setRight(x2);
                    continue;
                }
            }

            rects.append(QRectF(x1, m_trackY, w, m_trackHeight));
        }
    }

    m_rects = std::move(newRects);
    emit rectsChanged();
}

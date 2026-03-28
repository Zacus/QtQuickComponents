#include "TimelineModel.h"

#include <algorithm>
#include <QtCore/qdebug.h>

// ── 构造 ──────────────────────────────────────────────────────────────

TimelineModel::TimelineModel(QObject* parent)
    : QAbstractListModel(parent)
{}

// ── QAbstractListModel 接口 ───────────────────────────────────────────

int TimelineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;   // 平列模型，父节点无子行
    return m_segments.size();
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_segments.size())
        return {};

    const TimelineSegment& seg = m_segments.at(index.row());

    switch (role) {
    case StartMsRole:  return QVariant::fromValue(seg.startMs());
    case EndMsRole:    return QVariant::fromValue(seg.endMs());
    case TypeRole:     return seg.type();
    case DurationRole: return QVariant::fromValue(seg.durationMs());
    default:           return {};
    }
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    return {
        { StartMsRole,  "startMs"  },
        { EndMsRole,    "endMs"    },
        { TypeRole,     "type"     },
        { DurationRole, "duration" },
    };
}

// ── 整体范围 ──────────────────────────────────────────────────────────

qint64 TimelineModel::totalStart() const { return m_totalStart; }
qint64 TimelineModel::totalEnd()   const { return m_totalEnd;   }

// ── 写操作 ────────────────────────────────────────────────────────────

void TimelineModel::addSegment(qint64 startMs, qint64 endMs, int type)
{
    if (startMs >= endMs) {
        qWarning() << "TimelineModel::addSegment: invalid range"
                   << startMs << "->" << endMs << ", ignored";
        return;
    }

    TimelineSegment seg(startMs, endMs, type);
    const int pos = lowerBound(startMs);

    beginInsertRows({}, pos, pos);
    m_segments.insert(pos, seg);
    endInsertRows();

    updateBounds();
    emit countChanged();
}

void TimelineModel::addSegments(const QList<TimelineSegment>& segments)
{
    if (segments.isEmpty()) return;

    // 过滤无效区间
    QList<TimelineSegment> valid;
    valid.reserve(segments.size());
    for (const auto& s : segments) {
        if (s.isValid())
            valid.append(s);
        else
            qWarning() << "TimelineModel::addSegments: skipping invalid segment"
                       << s.startMs() << "->" << s.endMs();
    }
    if (valid.isEmpty()) return;

    // 排序新增部分
    std::sort(valid.begin(), valid.end());

    // 合并到现有列表（归并，保持整体有序）
    QList<TimelineSegment> merged;
    merged.reserve(m_segments.size() + valid.size());

    auto it1 = m_segments.cbegin();
    auto it2 = valid.cbegin();
    while (it1 != m_segments.cend() && it2 != valid.cend()) {
        if (*it1 < *it2)
            merged.append(*it1++);
        else
            merged.append(*it2++);
    }
    while (it1 != m_segments.cend()) merged.append(*it1++);
    while (it2 != valid.cend())      merged.append(*it2++);

    beginResetModel();
    m_segments = std::move(merged);
    endResetModel();

    updateBounds();
    emit countChanged();
}

void TimelineModel::removeSegment(int index)
{
    if (index < 0 || index >= m_segments.size()) return;

    beginRemoveRows({}, index, index);
    m_segments.removeAt(index);
    endRemoveRows();

    updateBounds();
    emit countChanged();
}

void TimelineModel::clear()
{
    if (m_segments.isEmpty()) return;

    beginResetModel();
    m_segments.clear();
    endResetModel();

    m_totalStart = 0;
    m_totalEnd   = 0;
    emit boundsChanged();
    emit countChanged();
}

void TimelineModel::mergeOverlapping(qint64 gapMs)
{
    // 列表已升序；扫一遍，相邻区间 type 相同且 end+gap >= nextStart 则合并
    if (m_segments.size() < 2) return;

    QList<TimelineSegment> result;
    result.reserve(m_segments.size());
    result.append(m_segments.first());

    for (int i = 1; i < m_segments.size(); ++i) {
        const TimelineSegment& cur  = m_segments.at(i);
        TimelineSegment&       last = result.last();

        if (last.type() == cur.type() && last.endMs() + gapMs >= cur.startMs()) {
            // 扩展尾部
            if (cur.endMs() > last.endMs())
                last.setEndMs(cur.endMs());
        } else {
            result.append(cur);
        }
    }

    if (result.size() == m_segments.size()) return;  // 无变化，不发信号

    beginResetModel();
    m_segments = std::move(result);
    endResetModel();

    // bounds 不变（首尾没变），但 count 变了
    emit countChanged();
}

// ── 查询接口 ──────────────────────────────────────────────────────────

QVariantList TimelineModel::segmentsInRange(qint64 viewStart, qint64 viewEnd) const
{
    if (m_segments.isEmpty() || viewStart >= viewEnd) return {};

    QVariantList result;

    const int endIdx = lowerBound(viewEnd);

    for (int i = 0; i < endIdx; ++i) {
        const TimelineSegment& seg = m_segments.at(i);
        if (seg.endMs() > viewStart)
            result.append(QVariant::fromValue(seg));
    }

    return result;
}

int TimelineModel::segmentIndexAt(qint64 timeMs) const
{
    // 找第一个 startMs > timeMs 的位置，向前找覆盖 timeMs 的区间
    const int pos = lowerBound(timeMs + 1);  // 第一个 startMs > timeMs
    for (int i = pos - 1; i >= 0; --i) {
        const TimelineSegment& seg = m_segments.at(i);
        if (seg.startMs() <= timeMs && seg.endMs() > timeMs)
            return i;
        if (seg.endMs() <= timeMs)
            break;  // 更早的区间不可能覆盖 timeMs
    }
    return -1;
}

TimelineSegment TimelineModel::segmentAt(int index) const
{
    if (index < 0 || index >= m_segments.size())
        return {};
    return m_segments.at(index);
}

// ── 私有工具 ──────────────────────────────────────────────────────────

int TimelineModel::lowerBound(qint64 startMs) const
{
    // 返回第一个 m_segments[i].startMs() >= startMs 的下标
    // 等价于 std::lower_bound，手写避免 lambda + 临时对象开销
    int lo = 0, hi = m_segments.size();
    while (lo < hi) {
        const int mid = lo + (hi - lo) / 2;
        if (m_segments.at(mid).startMs() < startMs)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

void TimelineModel::updateBounds()
{
    if (m_segments.isEmpty()) {
        if (m_totalStart != 0 || m_totalEnd != 0) {
            m_totalStart = 0;
            m_totalEnd   = 0;
            emit boundsChanged();
        }
        return;
    }

    // 列表有序，首元素 startMs 最小，但 endMs 最大值需要扫一遍
    // （允许存在包含关系的区间，endMs 不一定单调）
    const qint64 newStart = m_segments.first().startMs();
    qint64 newEnd = 0;
    for (const auto& seg : m_segments)
        newEnd = qMax(newEnd, seg.endMs());

    if (newStart != m_totalStart || newEnd != m_totalEnd) {
        m_totalStart = newStart;
        m_totalEnd   = newEnd;
        emit boundsChanged();
    }
}

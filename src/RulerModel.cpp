#include "RulerModel.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdatetime.h>

// ── 刻度级别表（静态，进程生命周期内只初始化一次）────────────────────
//
// 按 spanThreshold 从大到小排列。selectLevel() 从头扫描，
// 找到第一个 viewSpan > threshold 的条目即为当前级别。
// 最后一条 threshold=0 是兜底，viewSpan 再小也能匹配。
//
// 时间常量（ms）：
//   1s   = 1 000
//   1min = 60 000
//   1h   = 3 600 000
//   1d   = 86 400 000

const QList<RulerModel::Level> RulerModel::s_levels = {
    //  majorMs              minorMs           format          spanThreshold
    { 86400000LL,            6*3600000LL,      "MM-dd",        30*86400000LL  }, // >30天  → 天/6h
    {  6*3600000LL,            3600000LL,      "MM-dd HH:mm",   3*86400000LL  }, // >3天   → 6h/1h
    {    3600000LL,          15*60000LL,       "HH:mm",        12*3600000LL   }, // >12h   → 1h/15min
    {  15*60000LL,            5*60000LL,       "HH:mm",         2*3600000LL   }, // >2h    → 15min/5min
    {   5*60000LL,              60000LL,       "HH:mm",        30*60000LL     }, // >30min → 5min/1min
    {     60000LL,           15*1000LL,        "HH:mm:ss",      5*60000LL     }, // >5min  → 1min/15s
    {  15*1000LL,             5*1000LL,        "mm:ss",            60000LL    }, // >1min  → 15s/5s
    {   5*1000LL,               1000LL,        "mm:ss",         15*1000LL     }, // >15s   → 5s/1s
    {     1000LL,                200LL,        "ss.zzz",         3*1000LL     }, // >3s    → 1s/200ms
    {      200LL,                100LL,        "ss.zzz",           600LL      }, // >600ms → 200ms/100ms
    {      100LL,                 50LL,        "ss.zzz",             0LL      }, // ≤600ms → 100ms/50ms（兜底）
};

// ── 构造 ──────────────────────────────────────────────────────────────

RulerModel::RulerModel(QObject* parent)
    : QAbstractListModel(parent)
{}

// ── QAbstractListModel ────────────────────────────────────────────────

int RulerModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_ticks.size();
}

QVariant RulerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_ticks.size())
        return {};

    const RulerTick& tick = m_ticks.at(index.row());
    switch (role) {
    case TimeMsRole:  return QVariant::fromValue(tick.timeMs());
    case IsMajorRole: return tick.isMajor();
    case LabelRole:   return tick.label();
    default:          return {};
    }
}

QHash<int, QByteArray> RulerModel::roleNames() const
{
    return {
        { TimeMsRole,  "timeMs"  },
        { IsMajorRole, "isMajor" },
        { LabelRole,   "label"   },
    };
}

// ── viewport 属性 ─────────────────────────────────────────────────────

void RulerModel::setViewport(TimelineViewport* vp)
{
    if (m_viewport == vp) return;

    if (m_viewport)
        disconnect(m_viewport, &TimelineViewport::viewChanged,
                   this, &RulerModel::onViewChanged);

    m_viewport = vp;

    if (m_viewport)
        connect(m_viewport, &TimelineViewport::viewChanged,
                this, &RulerModel::onViewChanged);

    emit viewportChanged();
    rebuild();   // 立即用新视口重建刻度
}

// ── 公开查询 ──────────────────────────────────────────────────────────

RulerTick RulerModel::tickAt(int index) const
{
    if (index < 0 || index >= m_ticks.size()) return {};
    return m_ticks.at(index);
}

// ── 槽：视口变化时重建 ────────────────────────────────────────────────

void RulerModel::onViewChanged()
{
    rebuild();
}

// ── 核心算法 ──────────────────────────────────────────────────────────

void RulerModel::rebuild()
{
    if (!m_viewport) {
        if (!m_ticks.isEmpty()) {
            beginResetModel();
            m_ticks.clear();
            endResetModel();
            emit ticksChanged();
        }
        return;
    }

    const qint64 viewStart = m_viewport->viewStart();
    const qint64 viewEnd   = m_viewport->viewEnd();
    const qint64 viewSpan  = viewEnd - viewStart;

    if (viewSpan <= 0) return;

    // 1. 选择刻度级别
    const Level& lv = selectLevel(viewSpan);

    // 2. 更新派生属性（不需要发信号，ticksChanged 会整体通知）
    m_majorInterval = lv.majorMs;
    m_minorInterval = lv.minorMs;
    m_majorFormat   = lv.format;

    // 3. 生成副刻度列表
    //    从 alignDown(viewStart, minorMs) 开始，步进 minorMs，到 viewEnd 结束
    QList<RulerTick> newTicks;
    newTicks.reserve(qMin(
        static_cast<int>((viewSpan / lv.minorMs) + 2),
        kMaxTicks
    ));

    const qint64 firstMinor = alignDown(viewStart, lv.minorMs);

    for (qint64 t = firstMinor; t <= viewEnd; t += lv.minorMs) {
        if (newTicks.size() >= kMaxTicks) {
            qWarning() << "RulerModel: tick count exceeded kMaxTicks ("
                       << kMaxTicks << "), truncating. viewSpan=" << viewSpan
                       << "minorMs=" << lv.minorMs;
            break;
        }

        // 判断是否是主刻度（时间点是主刻度间隔的整数倍）
        const bool major = (t % lv.majorMs == 0);
        const QString label = major ? formatLabel(t, lv.format) : QString{};

        newTicks.append(RulerTick(t, major, label));
    }

    // 4. 只在实际变化时发 reset（避免视口平移但级别不变时无谓重绘）
    //    轻量比较：数量或首尾时间有变化就认为变化了
    const bool changed = (newTicks.size() != m_ticks.size())
        || (!newTicks.isEmpty() && !m_ticks.isEmpty()
            && (newTicks.first().timeMs() != m_ticks.first().timeMs()
                || newTicks.last().timeMs()  != m_ticks.last().timeMs()));

    if (!changed) return;

    beginResetModel();
    m_ticks = std::move(newTicks);
    endResetModel();

    emit ticksChanged();
}

// ── 私有工具 ──────────────────────────────────────────────────────────

const RulerModel::Level& RulerModel::selectLevel(qint64 viewSpan) const
{
    // s_levels 按 spanThreshold 降序排列，找第一个 viewSpan > threshold
    for (const Level& lv : s_levels) {
        if (viewSpan > lv.spanThreshold)
            return lv;
    }
    // 理论上 spanThreshold=0 的兜底条目一定能匹配，这里只防御性返回最后一个
    return s_levels.last();
}

qint64 RulerModel::alignDown(qint64 timeMs, qint64 intervalMs)
{
    if (intervalMs <= 0) return timeMs;

    // 天级别（≥ 1 天）需要以 UTC 午夜对齐，不能简单取余。
    // 对于 < 1 天的间隔，UTC epoch（1970-01-01 00:00:00 UTC）本身就在整点，
    // 直接取余即可得到对齐的整点时间。
    //
    // 之所以能这样：
    //   epoch = 0ms，24h = 86400000ms，整小时 = 3600000ms 的整数倍，
    //   所以 floor(timeMs / intervalMs) * intervalMs 就是最近的向下整点。

    if (timeMs >= 0) {
        return (timeMs / intervalMs) * intervalMs;
    } else {
        // 负时间（epoch 之前）：C++ 整数除法向零截断，需要手动向下取整
        return ((timeMs - intervalMs + 1) / intervalMs) * intervalMs;
    }
}

QString RulerModel::formatLabel(qint64 timeMs, const QString& fmt) const
{
    // 用 UTC 时间格式化（和 alignDown 保持一致）。
    // 如果业务需要本地时间，在这里替换为 QDateTime::fromMSecsSinceEpoch(t, Qt::LocalTime)。
    const QDateTime dt = QDateTime::fromMSecsSinceEpoch(timeMs, Qt::UTC);
    return dt.toString(fmt);
}

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
    //  majorMs                     minorMs               format           spanThreshold
    // ── 年 / 季 / 月 级别 ────────────────────────────────────────────────────────────
    { 365LL*86400000,   90LL*86400000,  "yyyy",        3LL*365*86400000 }, // >3年    → 年/季
    {  90LL*86400000,   30LL*86400000,  "yyyy-MM",       365*86400000LL }, // >1年    → 季/月
    {  30LL*86400000,    7LL*86400000,  "yyyy-MM",    90LL*86400000      }, // >90天   → 月/周
    {  14LL*86400000,    2LL*86400000,  "MM-dd",       30LL*86400000     }, // >30天   → 双周/2天
    // ── 周 / 天 级别 ─────────────────────────────────────────────────────────────────
    {   7LL*86400000,       86400000LL, "MM-dd",       14LL*86400000     }, // >14天   → 周/天
    {   3LL*86400000,    6*3600000LL,   "MM-dd",        7LL*86400000     }, // >7天    → 3天/6h
    {      86400000LL,    6*3600000LL,  "MM-dd",        3LL*86400000     }, // >3天    → 天/6h
    {      6*3600000LL,    3600000LL,   "MM-dd HH:mm",  86400000LL       }, // >1天    → 6h/1h
    // ── 小时 ~ 毫秒级别 ──────────────────────────────────────────────────────────────
    {        3600000LL,  15*60000LL,    "HH:mm",        12*3600000LL     }, // >12h    → 1h/15min
    {      15*60000LL,    5*60000LL,    "HH:mm",         2*3600000LL     }, // >2h     → 15min/5min
    {       5*60000LL,      60000LL,    "HH:mm",        30*60000LL       }, // >30min  → 5min/1min
    {         60000LL,   15*1000LL,     "HH:mm:ss",      5*60000LL       }, // >5min   → 1min/15s
    {       15*1000LL,    5*1000LL,     "mm:ss",            60000LL      }, // >1min   → 15s/5s
    {        5*1000LL,      1000LL,     "mm:ss",         15*1000LL       }, // >15s    → 5s/1s
    {          1000LL,       200LL,     "ss.zzz",         3*1000LL       }, // >3s     → 1s/200ms
    {           200LL,       100LL,     "ss.zzz",           600LL        }, // >600ms  → 200ms/100ms
    {           100LL,        50LL,     "ss.zzz",             0LL        }, // ≤600ms  → 100ms/50ms
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

    // 1. 选择刻度级别（同时考虑时间跨度和像素密度）
    const qreal viewWidth = m_viewport->viewWidth();
    const Level& lv = selectLevel(viewSpan, viewWidth);

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

const RulerModel::Level& RulerModel::selectLevel(qint64 viewSpan, qreal viewWidth,
                                                   qreal minPixelsPerMajor) const
{
    // 选择策略：从最粗粒度（天）向最细粒度（50ms）扫描，
    // 记录最后一个"主刻度像素间距 >= minPixelsPerMajor"的级别。
    //
    // 主刻度像素间距 = viewWidth * majorMs / viewSpan
    //
    // 从粗到细扫，取最后一个满足阈值的级别：
    //   → 视口很宽（缩小）时：只有粗级别满足，取粗级别
    //   → 视口适中时：粗、中级别都满足，取最细的那个满足的
    //   → 视口很窄（放大）时：细级别间距大，取细级别
    // 这样在任何缩放下都选"最细但不重叠"的级别。

    if (viewWidth <= 0.0 || viewSpan <= 0) return s_levels.last();

    const Level* best = &s_levels.first();  // 兜底：最粗级别
    for (const Level& lv : s_levels) {
        const qreal pixelsPerMajor =
            static_cast<qreal>(lv.majorMs) * viewWidth / static_cast<qreal>(viewSpan);
        if (pixelsPerMajor >= minPixelsPerMajor)
            best = &lv;   // 满足就记录，继续往细走
        else
            break;        // 一旦不满足，后面更细的也不会满足，提前退出
    }

    return *best;
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

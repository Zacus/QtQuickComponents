#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QList>
#include <QDateTime>

#include "RulerTick.h"
#include "TimelineViewport.h"

/**
 * @brief 时间轴刻度模型
 *
 * 监听 TimelineViewport 的 viewChanged 信号，视口变化时自动重新计算
 * 刻度列表并通知渲染层刷新，渲染层（Canvas / Repeater）只需读取此模型。
 *
 * === 刻度级别体系 ===
 *
 * 根据当前视口时间跨度（viewSpan）自动选择最合适的主/副刻度间隔：
 *
 *   viewSpan        主刻度间隔    副刻度间隔    标签格式
 *   ─────────────── ─────────── ─────────── ──────────────
 *   > 30 天         1 天         6 小时       MM-dd
 *   > 3 天          6 小时       1 小时       MM-dd HH:mm
 *   > 12 小时       1 小时       15 分钟      HH:mm
 *   > 2 小时        15 分钟      5 分钟       HH:mm
 *   > 30 分钟       5 分钟       1 分钟       HH:mm
 *   > 5 分钟        1 分钟       15 秒        HH:mm:ss
 *   > 1 分钟        15 秒        5 秒         mm:ss
 *   > 15 秒         5 秒         1 秒         mm:ss
 *   > 3 秒          1 秒         200 毫秒     ss.SSS
 *   > 600 毫秒      200 毫秒     100 毫秒     ss.SSS
 *   ≤ 600 毫秒      100 毫秒     50 毫秒      ss.SSS
 *
 * === 刻度对齐策略 ===
 *
 * 刻度线对齐到"自然边界"，而不是从 viewStart 起算：
 *   - 天级别：对齐到 UTC 午夜
 *   - 小时级别：对齐到整小时
 *   - 分钟级别：对齐到整分钟（以步长对齐，如 5 分钟步长对齐到 0/5/10...）
 *   - 秒/毫秒级别：同理
 *
 * 这样在缩放/平移时刻度不会漂移，始终停在"整点"上。
 *
 * === 性能设计 ===
 *
 * 只计算视口内的刻度，即 [viewStart, viewEnd] 范围内。
 * 最大刻度数量限制为 kMaxTicks（500），防止极端缩放时生成过多刻度。
 *
 * === QML 用法 ===
 * @code
 *   RulerModel {
 *       id: ruler
 *       viewport: myViewport   // 绑定后自动随视口更新
 *   }
 *
 *   // 在 Canvas.onPaint 中直接消费：
 *   for (var i = 0; i < ruler.count; ++i) {
 *       var tick = ruler.tickAt(i)
 *       var x = viewport.timeToPixel(tick.timeMs)
 *       ctx.moveTo(x, tick.isMajor ? 0 : 8)
 *       ctx.lineTo(x, tick.isMajor ? 20 : 14)
 *       if (tick.isMajor) ctx.fillText(tick.label, x, 28)
 *   }
 * @endcode
 */
class RulerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(TimelineViewport* viewport READ viewport
               WRITE setViewport NOTIFY viewportChanged)

    Q_PROPERTY(int     count         READ rowCount    NOTIFY ticksChanged)
    Q_PROPERTY(QString majorFormat   READ majorFormat NOTIFY ticksChanged)
    Q_PROPERTY(qint64  majorInterval READ majorInterval NOTIFY ticksChanged)
    Q_PROPERTY(qint64  minorInterval READ minorInterval NOTIFY ticksChanged)

public:
    // 单次最多生成刻度数，防止极端缩放
    static constexpr int kMaxTicks = 500;

    // ── 角色 ─────────────────────────────────────────────────
    enum Roles {
        TimeMsRole  = Qt::UserRole + 1,
        IsMajorRole,
        LabelRole,
    };
    Q_ENUM(Roles)

    explicit RulerModel(QObject* parent = nullptr);

    // ── QAbstractListModel ────────────────────────────────────
    int      rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ── viewport 属性 ─────────────────────────────────────────
    TimelineViewport* viewport() const { return m_viewport; }
    void setViewport(TimelineViewport* vp);

    // ── 派生信息（渲染层可直接读，不用遍历列表）──────────────
    QString majorFormat()   const { return m_majorFormat;   }
    qint64  majorInterval() const { return m_majorInterval; }
    qint64  minorInterval() const { return m_minorInterval; }

    /** 按索引取刻度（Canvas 绘制时比 data() 更直接）*/
    Q_INVOKABLE RulerTick tickAt(int index) const;

    /** 只读访问底层列表（渲染层迭代用）*/
    const QList<RulerTick>& ticks() const { return m_ticks; }

signals:
    void viewportChanged();
    void ticksChanged();

private slots:
    void onViewChanged();

private:
    // ── 刻度级别描述 ─────────────────────────────────────────
    struct Level {
        qint64  majorMs;       // 主刻度间隔（ms）
        qint64  minorMs;       // 副刻度间隔（ms）
        QString format;        // 主刻度标签格式（Qt::ISODate 子集）
        qint64  spanThreshold; // 当 viewSpan > threshold 时使用此级别
    };

    // minPixelsPerMajor: 主刻度最小像素间距，默认 100px
    // 100px 足够放下最长标签"MM-dd HH:mm"（约 80px at 11px font）加左右各 10px 边距
    const Level& selectLevel(qint64 viewSpan, qreal viewWidth,
                             qreal minPixelsPerMajor = 100.0) const;

    // 将时间 t 向下对齐到 interval 的整数倍（以 UTC epoch 为基准）
    static qint64 alignDown(qint64 timeMs, qint64 intervalMs);

    // 根据级别和视口范围重建 m_ticks
    void rebuild();

    // 格式化主刻度标签
    QString formatLabel(qint64 timeMs, const QString& fmt) const;

    TimelineViewport*   m_viewport      = nullptr;
    QList<RulerTick>    m_ticks;
    QString             m_majorFormat;
    qint64              m_majorInterval = 0;
    qint64              m_minorInterval = 0;

    // 刻度级别表（按 spanThreshold 降序排列，selectLevel 从头扫描）
    static const QList<Level> s_levels;
};

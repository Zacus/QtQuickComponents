#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QList>
#include <QVariant>

#include "TimelineSegment.h"

/**
 * @brief 时间轴录像/事件数据模型
 *
 * 继承 QAbstractListModel，可直接绑定到 QML 的任何 model 属性，
 * 也可通过 C++ 接口批量操作。
 *
 * === 数据约定 ===
 * - 内部始终保持按 startMs **升序**排列，便于二分查找。
 * - 不自动合并重叠区间——业务层如需合并请在 addSegments() 前处理，
 *   或调用 mergeOverlapping()。
 * - 时间单位：毫秒（ms），与 TimelineSegment 一致。
 *
 * === QML 用法 ===
 * @code
 *   TimelineView {
 *       model: myTimelineModel   // C++ 端通过 context / QML_ELEMENT 注入
 *       totalStart: myTimelineModel.totalStart
 *       totalEnd:   myTimelineModel.totalEnd
 *   }
 * @endcode
 *
 * === 角色说明 ===
 *   startMs  — qint64，区间起始毫秒
 *   endMs    — qint64，区间结束毫秒
 *   type     — int，片段类型（0=普通，1=移动侦测，2=报警）
 *   duration — qint64，区间时长毫秒（派生，只读）
 */
class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    // ── 整体时间范围（自动随数据更新）───────────────────────
    Q_PROPERTY(qint64 totalStart READ totalStart NOTIFY boundsChanged)
    Q_PROPERTY(qint64 totalEnd   READ totalEnd   NOTIFY boundsChanged)
    Q_PROPERTY(int    count      READ rowCount   NOTIFY countChanged)

public:
    // ── 角色枚举 ─────────────────────────────────────────────
    enum Roles {
        StartMsRole = Qt::UserRole + 1,
        EndMsRole,
        TypeRole,
        DurationRole
    };
    Q_ENUM(Roles)

    explicit TimelineModel(QObject* parent = nullptr);

    // ── QAbstractListModel 接口 ───────────────────────────────
    int      rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ── 整体范围访问器 ────────────────────────────────────────
    qint64 totalStart() const;   // 所有区间中最早的 startMs，无数据时返回 0
    qint64 totalEnd()   const;   // 所有区间中最晚的 endMs，无数据时返回 0

    // ── 写操作（均会维护升序、发射必要信号）─────────────────

    /**
     * 追加单条区间。区间无效（startMs >= endMs）时忽略。
     * 插入后保持升序（二分定位插入位置）。
     */
    Q_INVOKABLE void addSegment(qint64 startMs, qint64 endMs, int type = 0);

    /**
     * 批量追加，内部排序后一次性 insert，只发射一次 modelReset。
     * 适合初始化或大量数据导入场景，比循环 addSegment() 性能好得多。
     */
    Q_INVOKABLE void addSegments(const QList<TimelineSegment>& segments);

    /** 移除指定索引的区间。索引越界时忽略。 */
    Q_INVOKABLE void removeSegment(int index);

    /** 清空所有数据。 */
    Q_INVOKABLE void clear();

    /**
     * 合并重叠或相邻（间距 ≤ gapMs）的同 type 区间。
     * 可在批量导入后调用一次，减少渲染时的遍历量。
     */
    Q_INVOKABLE void mergeOverlapping(qint64 gapMs = 0);

    // ── 查询接口（渲染层高频调用，inline 实现减少调用开销）──

    /**
     * 返回与视口 [viewStart, viewEnd] 有交集的区间列表。
     * 使用二分查找，时间复杂度 O(log N + K)，K 为结果数量。
     * 渲染层每帧调用此方法获取需要绘制的区间，不遍历全量数据。
     *
     * 返回 QVariantList（每项为 QVariant 包装的 TimelineSegment），
     * QML 侧可直接当 JS 数组使用：segs[i].startMs / segs[i].endMs / segs[i].type
     */
    Q_INVOKABLE QVariantList segmentsInRange(qint64 viewStart, qint64 viewEnd) const;

    /**
     * 返回包含时间点 t 的第一个区间的索引，不存在返回 -1。
     * 用于点击命中检测。
     */
    Q_INVOKABLE int segmentIndexAt(qint64 timeMs) const;

    /** 按索引取区间（越界返回默认构造的空区间）。*/
    Q_INVOKABLE TimelineSegment segmentAt(int index) const;

    /** 只读访问底层列表（给渲染层直接迭代用，避免逐元素 data() 调用）。*/
    const QList<TimelineSegment>& segments() const { return m_segments; }

signals:
    void boundsChanged();
    void countChanged();

private:
    // 找升序插入位置（返回第一个 startMs >= seg.startMs 的下标）
    int lowerBound(qint64 startMs) const;

    // 重新计算并在变化时发射 boundsChanged
    void updateBounds();

    QList<TimelineSegment> m_segments;
    qint64 m_totalStart = 0;
    qint64 m_totalEnd   = 0;
};

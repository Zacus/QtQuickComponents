#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QtCore/qmath.h>

/**
 * @brief 时间轴视口状态与坐标变换
 *
 * 职责：
 *   1. 持有当前"看的是哪段时间"：viewStart / viewEnd（ms）
 *   2. 提供时间 ↔ 像素的双向换算
 *   3. 响应缩放、平移操作，并保证边界合法
 *
 * 所有渲染层（轨道、刻度尺、游标）都只读此对象的属性，
 * 不直接持有任何时间状态——视口是唯一的"坐标真相来源"。
 *
 * === 坐标换算公式 ===
 *   pixel  = (timeMs - viewStart) * pixelsPerMs
 *   timeMs = viewStart + pixel / pixelsPerMs
 *
 * === 缩放锚点语义 ===
 *   缩放时保持锚点时间不变：屏幕上 anchorX 像素对应的时间
 *   在缩放前后不移动。公式推导：
 *     anchorTime = viewStart + anchorX / pixelsPerMs_old
 *     viewStart' = anchorTime - anchorX / pixelsPerMs_new
 *     viewEnd'   = viewStart' + viewWidth / pixelsPerMs_new
 *
 * === QML 用法 ===
 * @code
 *   TimelineViewport {
 *       id: viewport
 *       viewWidth:  timelineCanvas.width   // 绑定画布宽度
 *       totalStart: model.totalStart
 *       totalEnd:   model.totalEnd
 *   }
 *
 *   // 渲染层读坐标
 *   x: viewport.timeToPixel(segment.startMs)
 *   width: viewport.durationToPixels(segment.durationMs)
 *
 *   // 交互层写视口
 *   WheelHandler { onWheel: viewport.zoomAt(point.x, delta > 0 ? 1.15 : 1/1.15) }
 * @endcode
 */
class TimelineViewport : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    // ── 缩放极限常量（ms）────────────────────────────────────
    // 最小视口宽度：1000ms = 1秒
    static constexpr qint64 kMinViewSpan =        1000LL;
    // 最大视口宽度：5年（覆盖年级别刻度，超过5年的数据用 fitAll 查看全貌）
    static constexpr qint64 kMaxViewSpan = 5LL * 365 * 24 * 3600 * 1000;

    explicit TimelineViewport(QObject* parent = nullptr);

    // ════════════════════════════════════════════════════════
    // 属性声明
    // ════════════════════════════════════════════════════════

    // ── 视口边界（ms）────────────────────────────────────────
    Q_PROPERTY(qint64 viewStart READ viewStart WRITE setViewStart NOTIFY viewChanged)
    Q_PROPERTY(qint64 viewEnd   READ viewEnd   WRITE setViewEnd   NOTIFY viewChanged)

    // ── 视口时间跨度（ms，派生）──────────────────────────────
    Q_PROPERTY(qint64 viewSpan  READ viewSpan  NOTIFY viewChanged)

    // ── 画布像素宽度（必须绑定到渲染层实际宽度）─────────────
    Q_PROPERTY(qreal viewWidth READ viewWidth WRITE setViewWidth NOTIFY viewWidthChanged)

    // ── 每毫秒像素数（派生，渲染层核心系数）─────────────────
    Q_PROPERTY(qreal pixelsPerMs READ pixelsPerMs NOTIFY viewChanged)

    // ── 数据总范围（ms，由 TimelineModel 绑定）───────────────
    Q_PROPERTY(qint64 totalStart READ totalStart WRITE setTotalStart NOTIFY totalRangeChanged)
    Q_PROPERTY(qint64 totalEnd   READ totalEnd   WRITE setTotalEnd   NOTIFY totalRangeChanged)

    // ── 缩放系数上下限（可由外部覆盖）────────────────────────
    Q_PROPERTY(qreal minZoomFactor READ minZoomFactor WRITE setMinZoomFactor NOTIFY limitsChanged)
    Q_PROPERTY(qreal maxZoomFactor READ maxZoomFactor WRITE setMaxZoomFactor NOTIFY limitsChanged)

    // ════════════════════════════════════════════════════════
    // 访问器
    // ════════════════════════════════════════════════════════

    qint64 viewStart()    const { return m_viewStart; }
    qint64 viewEnd()      const { return m_viewEnd;   }
    qint64 viewSpan()     const { return m_viewEnd - m_viewStart; }
    qreal  viewWidth()    const { return m_viewWidth; }
    qreal  pixelsPerMs()  const { return m_pixelsPerMs; }
    qint64 totalStart()   const { return m_totalStart; }
    qint64 totalEnd()     const { return m_totalEnd;   }
    qreal  minZoomFactor() const { return m_minZoomFactor; }
    qreal  maxZoomFactor() const { return m_maxZoomFactor; }

    void setViewStart    (qint64 v);
    void setViewEnd      (qint64 v);
    void setViewWidth    (qreal  v);
    void setTotalStart   (qint64 v);
    void setTotalEnd     (qint64 v);
    void setMinZoomFactor(qreal  v);
    void setMaxZoomFactor(qreal  v);

    // ════════════════════════════════════════════════════════
    // 坐标换算（高频调用，inline 实现）
    // ════════════════════════════════════════════════════════

    /**
     * 时间（ms）→ 像素 X 坐标
     * 结果可能在 [0, viewWidth] 之外（超出视口），调用方按需裁剪。
     */
    Q_INVOKABLE inline qreal timeToPixel(qint64 timeMs) const
    {
        return static_cast<qreal>(timeMs - m_viewStart) * m_pixelsPerMs;
    }

    /**
     * 像素 X 坐标 → 时间（ms）
     * 反向换算，用于鼠标点击命中检测、拖拽计算。
     */
    Q_INVOKABLE inline qint64 pixelToTime(qreal px) const
    {
        if (m_pixelsPerMs <= 0.0) return m_viewStart;
        return m_viewStart + static_cast<qint64>(px / m_pixelsPerMs);
    }

    /**
     * 时长（ms）→ 像素宽度
     * 负时长返回 0，调用方不需要判断。
     */
    Q_INVOKABLE inline qreal durationToPixels(qint64 durationMs) const
    {
        if (durationMs <= 0) return 0.0;
        return static_cast<qreal>(durationMs) * m_pixelsPerMs;
    }

    /**
     * 像素宽度 → 时长（ms）
     * 用于从像素偏移量推算时间偏移量（拖拽平移）。
     */
    Q_INVOKABLE inline qint64 pixelsToDuration(qreal px) const
    {
        if (m_pixelsPerMs <= 0.0) return 0;
        return static_cast<qint64>(px / m_pixelsPerMs);
    }

    /**
     * 判断时间点是否在当前视口内（含边界）
     */
    Q_INVOKABLE inline bool isVisible(qint64 timeMs) const
    {
        return timeMs >= m_viewStart && timeMs <= m_viewEnd;
    }

    /**
     * 判断区间 [start, end] 是否与视口有交集
     */
    Q_INVOKABLE inline bool isRangeVisible(qint64 startMs, qint64 endMs) const
    {
        return startMs < m_viewEnd && endMs > m_viewStart;
    }

    // ════════════════════════════════════════════════════════
    // 视口操作（供交互层调用）
    // ════════════════════════════════════════════════════════

    /**
     * 以屏幕像素位置 anchorPx 为锚点缩放。
     *
     * @param anchorPx  锚点像素（通常是鼠标 X 坐标）
     * @param factor    缩放因子：> 1 放大（时间跨度缩小），< 1 缩小
     *
     * 锚点时间在缩放前后保持不变，即屏幕上该像素位置对应的时刻不移动。
     */
    Q_INVOKABLE void zoomAt(qreal anchorPx, qreal factor);

    /**
     * 平移视口（正数向右/未来方向，负数向左/过去方向）。
     *
     * @param deltaPixels  像素偏移量（通常是鼠标拖拽的 dx）
     */
    Q_INVOKABLE void panBy(qreal deltaPixels);

    /**
     * 直接跳转到指定时间点，并将其置于视口左端。
     * 不改变缩放级别（viewSpan 不变）。
     */
    Q_INVOKABLE void scrollToTime(qint64 timeMs);

    /**
     * 将指定时间点居中显示。
     * 不改变缩放级别。
     */
    Q_INVOKABLE void centerOnTime(qint64 timeMs);

    /**
     * 将视口适配到指定时间范围（同时调整 viewStart、viewEnd）。
     * 会受 kMinViewSpan / kMaxViewSpan 限制。
     */
    Q_INVOKABLE void fitRange(qint64 startMs, qint64 endMs);

    /**
     * 将视口适配到 totalStart ~ totalEnd 整个数据范围。
     */
    Q_INVOKABLE void fitAll();

signals:
    void viewChanged();         // viewStart / viewEnd / pixelsPerMs 任一变化
    void viewWidthChanged();    // 画布宽度变化
    void totalRangeChanged();   // totalStart / totalEnd 变化
    void limitsChanged();       // 缩放限制变化

private:
    // 重新计算 m_pixelsPerMs，在任何影响它的属性变化后调用
    void recalcPixelsPerMs();

    // 将 [start, end] 夹在合法范围内并写入成员变量
    // 合法性规则：
    //   1. span 在 [kMinViewSpan, kMaxViewSpan]
    //   2. viewStart >= totalStart（如果 totalStart != totalEnd，否则不限）
    //   3. viewEnd   <= totalEnd （同上）
    // 返回是否实际发生了变化
    bool applyView(qint64 start, qint64 end);

    // 仅夹边界，不检查 span（供 applyView 内部使用）
    void clampToBounds(qint64& start, qint64& end) const;

    qint64 m_viewStart    = 0;
    qint64 m_viewEnd      = 0;
    qreal  m_viewWidth    = 1.0;   // 默认 1px，防止 pixelsPerMs = 0
    qreal  m_pixelsPerMs  = 1.0;
    qint64 m_totalStart   = 0;
    qint64 m_totalEnd     = 0;
    qreal  m_minZoomFactor = 1.1;
    qreal  m_maxZoomFactor = 1.1;  // 仅用于 zoomAt 的单步限幅，不影响绝对范围
};

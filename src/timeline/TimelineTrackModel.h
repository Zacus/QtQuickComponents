#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QVariantList>
#include <QRectF>

#include "TimelineModel.h"
#include "TimelineViewport.h"

/**
 * @brief 像素级合并区间模型（渲染优化层）
 *
 * 职责：监听 TimelineModel 和 TimelineViewport 的变化，
 * 在 C++ 里预先计算出"当前视口内需要绘制的矩形列表"，
 * Canvas 的 onPaint 直接取结果绘制，不在 JS 里做坐标换算和合并。
 *
 * === 优化原理 ===
 *
 * 原始方案（TimelineTrack.qml）：
 *   onPaint → segmentsInRange() → JS 逐条换算像素 → fillRect × N
 *   问题：N 条区间全部在 JS 里处理，GC 压力大，每帧 O(K) JS 执行
 *
 * 优化方案：
 *   C++ 里把视口内区间换算为像素 x/width，相邻像素距离 < mergeGap 的合并，
 *   结果存为 QVariantList<QRectF>，onPaint 只需 drawRects(rects) 一次调用。
 *   JS 层零换算，零合并，只做最终绘制。
 *
 * === 合并规则 ===
 *
 *   同 type 的相邻区间，像素间距 < mergeGapPx（默认 1px）时合并为一个矩形。
 *   不同 type 的区间不合并（颜色不同）。
 *   合并后每种 type 独立输出一个矩形列表。
 *
 * === QML 用法 ===
 * @code
 *   TimelineTrackModel {
 *       id: trackModel
 *       model:    myTimelineModel
 *       viewport: _viewport
 *   }
 *
 *   Canvas {
 *       onPaint: {
 *           var ctx = getContext("2d")
 *           // 按 type 取矩形列表直接绘制
 *           var rects0 = trackModel.rectsForType(0)  // 普通录像
 *           ctx.fillStyle = "#4c6ef5"
 *           for (var i = 0; i < rects0.length; ++i) {
 *               var r = rects0[i]
 *               ctx.fillRect(r.x, r.y, r.width, r.height)
 *           }
 *       }
 *   }
 * @endcode
 */
class TimelineTrackModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(TimelineModel*    model    READ model    WRITE setModel    NOTIFY modelChanged)
    Q_PROPERTY(TimelineViewport* viewport READ viewport WRITE setViewport NOTIFY viewportChanged)

    // 合并阈值：像素间距 < mergeGapPx 的同 type 区间合并为一个矩形（默认 1px）
    Q_PROPERTY(qreal mergeGapPx  READ mergeGapPx  WRITE setMergeGapPx  NOTIFY mergeGapPxChanged)

    // 区间矩形的高度和 Y 偏移（通常和轨道高度一致，由外部绑定）
    Q_PROPERTY(qreal trackHeight READ trackHeight WRITE setTrackHeight NOTIFY trackHeightChanged)
    Q_PROPERTY(qreal trackY      READ trackY      WRITE setTrackY      NOTIFY trackYChanged)

    // 当前视口内有多少个合并后矩形（所有 type 合计，用于调试）
    Q_PROPERTY(int rectCount READ rectCount NOTIFY rectsChanged)

public:
    explicit TimelineTrackModel(QObject* parent = nullptr);

    TimelineModel*    model()    const { return m_model;    }
    TimelineViewport* viewport() const { return m_viewport; }
    qreal mergeGapPx()  const { return m_mergeGapPx;  }
    qreal trackHeight() const { return m_trackHeight; }
    qreal trackY()      const { return m_trackY;      }
    int   rectCount()   const;

    void setModel      (TimelineModel*    v);
    void setViewport   (TimelineViewport* v);
    void setMergeGapPx (qreal v);
    void setTrackHeight(qreal v);
    void setTrackY     (qreal v);

    /**
     * 返回指定 type 的合并矩形列表，供 Canvas onPaint 直接遍历绘制。
     * 返回 QVariantList，每个元素是 QVariant(QRectF)，
     * QML 侧用 r.x / r.y / r.width / r.height 访问。
     */
    Q_INVOKABLE QVariantList rectsForType(int type) const;

    /**
     * 返回所有 type 的矩形，以 [{type, x, y, width, height}, ...] 格式。
     * 适合不关心颜色分组、需要统一遍历的场景。
     */
    Q_INVOKABLE QVariantList allRects() const;

signals:
    void modelChanged();
    void viewportChanged();
    void mergeGapPxChanged();
    void trackHeightChanged();
    void trackYChanged();
    void rectsChanged();   // 矩形列表更新，Canvas 应 requestPaint()

private slots:
    void onViewChanged();
    void onDataChanged();

private:
    void rebuild();

    // 内部存储：type → 合并后的矩形列表
    // type 通常只有 0/1/2，用 QHash 避免稀疏 type 浪费空间
    QHash<int, QList<QRectF>> m_rects;

    TimelineModel*    m_model       = nullptr;
    TimelineViewport* m_viewport    = nullptr;
    qreal             m_mergeGapPx  = 1.0;
    qreal             m_trackHeight = 20.0;
    qreal             m_trackY      = 0.0;
};

#pragma once

#include <QtQml/qqml.h>
#include <QtCore/qdatetime.h>

/**
 * @brief 单条录像/事件区间
 *
 * 用作 TimelineModel 的元素类型，也可在 QML 侧直接构造：
 *   var seg = Qt.createQmlObject('...', ...)  // 通常由 C++ 端批量填充
 *
 * 时间单位统一使用 **毫秒（ms）**，以 qint64 存储，
 * 足以精确表示从 epoch 起超过 29 万年的范围，不存在精度问题。
 *
 * type 字段用于区分不同颜色/来源的片段：
 *   0 = 普通录像（默认）
 *   1 = 移动侦测触发
 *   2 = 报警触发
 *   （可按业务扩展，渲染层按 type 映射颜色）
 */
class TimelineSegment
{
    Q_GADGET
    QML_VALUE_TYPE(timelineSegment)

    Q_PROPERTY(qint64 startMs READ startMs WRITE setStartMs)
    Q_PROPERTY(qint64 endMs   READ endMs   WRITE setEndMs)
    Q_PROPERTY(int    type    READ type    WRITE setType)

public:
    TimelineSegment() = default;

    TimelineSegment(qint64 startMs, qint64 endMs, int type = 0)
        : m_startMs(startMs), m_endMs(endMs), m_type(type)
    {}

    // ── 访问器 ───────────────────────────────────────────────
    qint64 startMs() const { return m_startMs; }
    qint64 endMs()   const { return m_endMs;   }
    int    type()    const { return m_type;    }

    void setStartMs(qint64 v) { m_startMs = v; }
    void setEndMs  (qint64 v) { m_endMs   = v; }
    void setType   (int    v) { m_type    = v; }

    // ── 派生属性（只读工具方法，不注册 Q_PROPERTY 避免冗余）──
    qint64 durationMs() const { return m_endMs - m_startMs; }
    bool   isValid()    const { return m_startMs < m_endMs; }

    // ── 比较（用于 QList 排序 / 二分查找）──────────────────
    bool operator<(const TimelineSegment& o) const { return m_startMs < o.m_startMs; }
    bool operator==(const TimelineSegment& o) const
    {
        return m_startMs == o.m_startMs
            && m_endMs   == o.m_endMs
            && m_type    == o.m_type;
    }

    // ── 便捷工厂：从 QDateTime 构造（自动转 epoch ms）───────
    static TimelineSegment fromDateTimes(const QDateTime& start,
                                         const QDateTime& end,
                                         int type = 0)
    {
        return TimelineSegment(
            start.toMSecsSinceEpoch(),
            end.toMSecsSinceEpoch(),
            type
        );
    }

private:
    qint64 m_startMs = 0;
    qint64 m_endMs   = 0;
    int    m_type    = 0;
};

// 让 QVariant / QML 能识别此类型
Q_DECLARE_METATYPE(TimelineSegment)

#pragma once

#include <QtQml/qqml.h>
#include <QString>

/**
 * @brief 单条刻度的渲染数据
 *
 * 由 RulerModel 批量生成，渲染层（Canvas / Repeater）直接消费。
 * 使用 Q_GADGET + QML_VALUE_TYPE 而非 QObject，保持值语义，
 * 批量存储进 QList 时可做 memcpy 级别的移动，无堆分配开销。
 *
 * isMajor 区分主/副刻度：
 *   主刻度：长线 + 时间标签（如 "14:30"）
 *   副刻度：短线，无标签
 */
class RulerTick
{
    Q_GADGET
    QML_VALUE_TYPE(rulerTick)

    Q_PROPERTY(qint64  timeMs  READ timeMs  CONSTANT)
    Q_PROPERTY(bool    isMajor READ isMajor CONSTANT)
    Q_PROPERTY(QString label   READ label   CONSTANT)

public:
    RulerTick() = default;

    RulerTick(qint64 timeMs, bool isMajor, const QString& label = {})
        : m_timeMs(timeMs), m_isMajor(isMajor), m_label(label)
    {}

    qint64  timeMs()  const { return m_timeMs;  }
    bool    isMajor() const { return m_isMajor; }
    QString label()   const { return m_label;   }

    bool operator==(const RulerTick& o) const
    {
        return m_timeMs == o.m_timeMs && m_isMajor == o.m_isMajor;
    }

private:
    qint64  m_timeMs  = 0;
    bool    m_isMajor = false;
    QString m_label;
};

Q_DECLARE_METATYPE(RulerTick)

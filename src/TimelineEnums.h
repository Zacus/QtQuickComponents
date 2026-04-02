#pragma once

#include <QObject>
#include <QQmlEngine>

/**
 * @brief 时间轴组件枚举命名空间
 *
 * 通过 QML_SINGLETON 注册，在 QML 中以类型名直接访问枚举值，
 * 提供编译期检查，避免字符串拼写错误静默失败。
 *
 * QML 用法：
 * @code
 *   import QuickUI.Components 1.0
 *
 *   TimelineView {
 *       followMode: TimelineEnums.FollowEdge    // ✓ 类型安全
 *       // followMode: "edge"                   // ✗ 字符串，拼错不报错
 *   }
 * @endcode
 *
 * 枚举值可在 QML 条件中直接比较：
 * @code
 *   if (timeline.followMode === TimelineEnums.FollowEdge) { ... }
 * @endcode
 */
class TimelineEnums : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    // ── 游标跟随模式 ──────────────────────────────────────────
    enum FollowMode {
        FollowNone   = 0,  ///< 不自动滚动，用户手动平移
        FollowEdge   = 1,  ///< 游标到达视口边缘时平移（推荐，类似视频编辑软件）
        FollowCenter = 2,  ///< 视口始终居中于游标（类似音频软件）
    };
    Q_ENUM(FollowMode)

    // ── 区间片段类型（与 TimelineSegment::type 对应）─────────
    enum SegmentType {
        SegmentNormal   = 0,  ///< 普通录像
        SegmentMotion   = 1,  ///< 移动侦测触发
        SegmentAlarm    = 2,  ///< 报警触发
    };
    Q_ENUM(SegmentType)

    // ── 游标头位置 ────────────────────────────────────────────
    enum HeadPosition {
        HeadTop    = 0,  ///< 三角头在顶部（默认）
        HeadBottom = 1,  ///< 三角头在底部
    };
    Q_ENUM(HeadPosition)

    static TimelineEnums* create(QQmlEngine*, QJSEngine*)
    {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    static TimelineEnums& instance()
    {
        static TimelineEnums s;
        return s;
    }

private:
    explicit TimelineEnums(QObject* parent = nullptr) : QObject(parent) {}
};

import QtQuick
import QuickUI.Components 1.0

/**
 * 回放游标覆盖层（Playhead）
 *
 * 叠放在 TimelineTrack 和 TimelineInputHandler 之间：
 *
 *   z 层次（从下到上）：
 *     TimelineTrack       ← 录像区间色块
 *     PlayheadOverlay     ← 游标线 + 时间标签      ← 这里
 *     TimelineInputHandler← 轨道平移/缩放交互
 *
 * 组成：
 *   ┌────────────┐
 *   │  ▼ 三角头  │  ← headItem（默认在顶部）
 *   │  │ 竖线   │  ← lineItem
 *   │  "HH:mm:ss"│  ← timeLabel（悬浮在头部旁边）
 *   └────────────┘
 *
 * 拖拽语义：
 *   拖拽游标头（headItem）→ 发 seeked(timeMs)，修改 currentTime
 *   拖拽轨道空白区 → 由 TimelineInputHandler 处理（平移视口）
 *   两者互不干扰：游标头的 DragHandler 会 grabExclusive，
 *   InputHandler 的 DragHandler 不会被激活。
 *
 * 属性：
 *   viewport        — TimelineViewport（必须绑定）
 *   currentTime     — 游标时间 ms（双向，外部播放器可驱动）
 *   headPosition    — "top" | "bottom"，三角头位置
 *   lineColor       — 游标线颜色
 *   headColor       — 三角头颜色
 *   labelVisible    — 是否显示时间标签
 *   labelFormat     — 时间标签格式（Qt 日期格式字符串）
 *   autoScroll      — true 时游标接近视口边缘时自动平移视口
 *   autoScrollMargin— 触发自动滚动的边缘像素宽度
 *
 * 信号：
 *   seeked(timeMs)  — 用户拖拽游标后发出
 */
Item {
    id: root

    // ── 必须绑定 ─────────────────────────────────────────────
    required property var viewport     // TimelineViewport

    // ── 游标时间（双向）──────────────────────────────────────
    property real currentTime: 0       // ms

    // ── 外观 ─────────────────────────────────────────────────
    property string headPosition:  "top"       // "top" | "bottom"
    property color  lineColor:     "#ffffff"
    property real   lineWidth:     1.5
    property color  headColor:     "#ffffff"
    property real   headSize:      10          // 三角头边长（px）
    property bool   labelVisible:  true
    property string labelFormat:   "hh:mm:ss"  // Qt 时间格式，可覆盖为 "hh:mm:ss.zzz"
    property color  labelBg:       Qt.rgba(0, 0, 0, 0.65)
    property color  labelColor:    "#ffffff"
    property real   labelFontSize: ComponentTheme.fontSizeLabel

    // ── 自动滚动 ─────────────────────────────────────────────
    property bool   autoScroll:       true
    property real   autoScrollMargin: 40       // px

    // ── 信号 ─────────────────────────────────────────────────
    signal seeked(real timeMs)

    // ── 只读状态 ─────────────────────────────────────────────
    readonly property bool isDragging: _headDrag.active

    // ══════════════════════════════════════════════════════════
    // 派生计算（缓存，避免重复计算）
    // ══════════════════════════════════════════════════════════

    // 游标像素 X 坐标（视口之外时仍计算，由 visible 控制显隐）
    readonly property real _x: viewport ? viewport.timeToPixel(currentTime) : -1

    // 是否在视口内（含边距）
    readonly property bool _inView: viewport
        ? (_x >= -lineWidth && _x <= viewport.viewWidth + lineWidth)
        : false

    // 三角头是否在顶部
    readonly property bool _headAtTop: headPosition === "top"

    // ══════════════════════════════════════════════════════════
    // 子项布局
    // ══════════════════════════════════════════════════════════

    // 整体跟随游标 X，visible 控制在视口外时隐藏
    x:       _x - lineWidth / 2   // 竖线居中在 _x 上
    y:       0
    width:   lineWidth
    height:  parent ? parent.height : 0
    visible: _inView
    // 不拦截非游标区域的事件，让 InputHandler 正常工作
    // 游标头 Item 会自己 grabExclusive
    containmentMask: null

    // ── 竖线 ─────────────────────────────────────────────────
    Rectangle {
        id: lineItem
        anchors {
            top:    _headAtTop ? parent.top : undefined
            bottom: _headAtTop ? undefined  : parent.bottom
            topMargin:    _headAtTop ? root.headSize : 0
            bottomMargin: _headAtTop ? 0 : root.headSize
        }
        width:  root.lineWidth
        height: root.height - root.headSize
        color:  root.lineColor
        // 抗锯齿：整数像素对齐
        x: 0
    }

    // ── 三角头 ───────────────────────────────────────────────
    // 用 Canvas 画等腰三角形，尖端指向轨道方向
    Canvas {
        id: headItem
        width:  root.headSize * 2
        height: root.headSize
        // 居中对齐到游标线
        x: -root.headSize + root.lineWidth / 2
        y: _headAtTop ? 0 : root.height - root.headSize

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = root.headColor

            var w = width, h = height
            ctx.beginPath()
            if (root._headAtTop) {
                // 尖端朝下
                ctx.moveTo(0,     0)
                ctx.lineTo(w,     0)
                ctx.lineTo(w / 2, h)
            } else {
                // 尖端朝上
                ctx.moveTo(w / 2, 0)
                ctx.lineTo(w,     h)
                ctx.lineTo(0,     h)
            }
            ctx.closePath()
            ctx.fill()
        }

        // ── 游标头拖拽 ────────────────────────────────────────
        // 命中区域比视觉大，方便触屏操作
        property real _hitExpand: 8

        // 扩大命中区
        Item {
            id: _hitArea
            anchors.centerIn: parent
            width:  headItem.width  + headItem._hitExpand * 2
            height: headItem.height + headItem._hitExpand * 2

            DragHandler {
                id: _headDrag
                target: null
                acceptedButtons: Qt.LeftButton
                // grabExclusive 确保父层 InputHandler 的 DragHandler 不会同时激活
                grabPermissions: PointerHandler.CanTakeOverFromAnything

                property real _startX: 0

                onActiveChanged: {
                    if (active) {
                        _startX = centroid.position.x + root._x
                        cursorShape = Qt.SizeHorCursor
                    } else {
                        cursorShape = Qt.ArrowCursor
                    }
                }

                onCentroidChanged: {
                    if (!active || !root.viewport) return

                    // centroid 是相对于 _hitArea 的坐标
                    // 换算到轨道坐标系：加上游标当前 X
                    var absX = centroid.position.x + root._x
                    var t    = root.viewport.pixelToTime(absX)

                    // 夹在数据总范围内
                    var tClamped = Math.max(
                        root.viewport.totalStart,
                        Math.min(root.viewport.totalEnd, t)
                    )

                    root.currentTime = tClamped
                    root.seeked(tClamped)

                    // 自动滚动：游标拖到视口边缘时平移
                    if (root.autoScroll) {
                        var px = root.viewport.timeToPixel(tClamped)
                        var margin = root.autoScrollMargin
                        var vw = root.viewport.viewWidth
                        if (px < margin) {
                            root.viewport.panBy(px - margin)
                        } else if (px > vw - margin) {
                            root.viewport.panBy(px - (vw - margin))
                        }
                    }
                }

                cursorShape: Qt.SizeHorCursor
            }
        }

        Component.onCompleted: requestPaint()
    }

    // ── 时间标签 ─────────────────────────────────────────────
    Item {
        id: timeLabelContainer
        visible: root.labelVisible
        // 标签贴在三角头旁边，避免遮住轨道内容
        y: _headAtTop
            ? 0
            : root.height - root.headSize - labelBox.height - 2

        // 标签居中对齐游标线，但夹在画布边界内
        x: {
            var ideal = -labelBox.width / 2 + root.lineWidth / 2
            var minX  = -root._x + 2                           // 左边界
            var maxX  = -root._x + (root.viewport ? root.viewport.viewWidth : 0)
                        - labelBox.width - 2                   // 右边界
            return Math.max(minX, Math.min(maxX, ideal))
        }

        // 标签背景 + 文字
        Rectangle {
            id: labelBox
            color:  root.labelBg
            radius: 3
            width:  timeText.implicitWidth + 8
            height: timeText.implicitHeight + 4

            Text {
                id: timeText
                anchors.centerIn: parent
                text: {
                    if (!root.viewport) return ""
                    // 从 currentTime（ms since epoch）格式化
                    var dt = new Date(root.currentTime)
                    return Qt.formatTime(dt, root.labelFormat)
                }
                color:          root.labelColor
                font.pixelSize: root.labelFontSize
                font.family:    ComponentTheme.fontFamily
            }
        }
    }

    // ══════════════════════════════════════════════════════════
    // 视口变化时更新位置（x 是绑定表达式，自动响应）
    // viewport.viewChanged → _x 重新计算 → x 更新 → Item 移动
    // 无需额外 Connections，纯声明式绑定已经覆盖
    // ══════════════════════════════════════════════════════════

    // 颜色变化时重绘三角头
    onHeadColorChanged:   headItem.requestPaint()
    onHeadPositionChanged: headItem.requestPaint()
    onHeadSizeChanged:    headItem.requestPaint()
}

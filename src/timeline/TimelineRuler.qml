import QtQuick
import QuickUI.Components 1.0

/**
 * 时间轴刻度尺
 *
 * 职责：根据 RulerModel 提供的刻度列表，在 Canvas 上绘制：
 *   - 副刻度：短线，无标签
 *   - 主刻度：长线 + 时间标签
 *   - 可选底部分隔线
 *
 * 属性：
 *   viewport   — TimelineViewport，用于 timeToPixel 换算（必须绑定）
 *   rulerModel — RulerModel，刻度数据源（必须绑定）
 *
 * 布局（默认，可通过属性覆盖）：
 *
 *   ┌──────────────────────────────────────┐  ← y=0
 *   │  |    |    |    |    |    |    |     │  ← 副刻度线（minorTickHeight）
 *   │  |||  |    |||  |    |||  |    |||   │  ← 主刻度线（majorTickHeight）
 *   │ 14:00      14:15      14:30          │  ← 标签（labelY）
 *   └──────────────────────────────────────┘  ← height
 *
 * 刻度线从顶部向下画（tick 在上，label 在下），可通过 flip: true 翻转
 * （tick 在下，label 在上），适合刻度尺放在轨道下方的布局。
 */
Item {
    id: root

    // ── 必须绑定的外部依赖 ───────────────────────────────────
    required property var viewport     // TimelineViewport
    required property var rulerModel   // RulerModel

    // ── 外观定制 ─────────────────────────────────────────────
    property color  backgroundColor: "transparent"
    property color  majorTickColor:  ComponentTheme.textSecondary
    property color  minorTickColor:  Qt.rgba(
                        ComponentTheme.textSecondary.r,
                        ComponentTheme.textSecondary.g,
                        ComponentTheme.textSecondary.b, 0.4)
    property color  labelColor:      ComponentTheme.textSecondary
    property color  separatorColor:  ComponentTheme.separator

    property real   majorTickHeight: 12    // 主刻度线高度（px）
    property real   minorTickHeight:  6    // 副刻度线高度（px）
    property real   tickWidth:        1    // 刻度线宽度（px）
    property string labelFont:       "11px sans-serif"
    property bool   showSeparator:   true  // 底部分隔线
    property bool   flip:            false // true = 刻度在下，标签在上

    // ── Canvas ───────────────────────────────────────────────
    Canvas {
        id: canvas
        anchors.fill: parent
        renderStrategy: Canvas.Cooperative

        onPaint: {
            var ctx = getContext("2d")
            var w   = width
            var h   = height

            ctx.clearRect(0, 0, w, h)

            // 背景
            if (root.backgroundColor !== "transparent") {
                ctx.fillStyle = root.backgroundColor
                ctx.fillRect(0, 0, w, h)
            }

            if (!root.viewport || !root.rulerModel) return

            var vp         = root.viewport
            var ruler      = root.rulerModel
            var count      = ruler.count
            if (count === 0) return

            var flipped    = root.flip
            var majH       = root.majorTickHeight
            var minH       = root.minorTickHeight
            var tw         = root.tickWidth

            // Y 坐标计算（两种布局）
            // flip=false（刻度在上）: 线从 y=0 向下，标签在线底部再偏 2px
            // flip=true （刻度在下）: 线从 y=h 向上，标签在线顶部再偏 -2px
            var majLineY1, majLineY2, minLineY1, minLineY2, labelY, labelBaseline

            if (!flipped) {
                majLineY1    = 0
                majLineY2    = majH
                minLineY1    = 0
                minLineY2    = minH
                labelY       = majH + 2
                labelBaseline = "top"
            } else {
                majLineY1    = h
                majLineY2    = h - majH
                minLineY1    = h
                minLineY2    = h - minH
                labelY       = h - majH - 2
                labelBaseline = "bottom"
            }

            // 字体和标签对齐
            ctx.font         = root.labelFont
            ctx.textBaseline = labelBaseline
            ctx.textAlign    = "center"

            // ── 副刻度批量绘制 ────────────────────────────────
            ctx.beginPath()
            ctx.strokeStyle = root.minorTickColor
            ctx.lineWidth   = tw

            for (var i = 0; i < count; i++) {
                var tick = ruler.tickAt(i)
                if (tick.isMajor) continue

                var x = vp.timeToPixel(tick.timeMs)
                if (x < -1 || x > w + 1) continue  // 视口外跳过

                ctx.moveTo(x + 0.5, minLineY1)  // +0.5 避免亚像素模糊
                ctx.lineTo(x + 0.5, minLineY2)
            }
            ctx.stroke()

            // ── 主刻度批量绘制（线 + 标签）────────────────────
            ctx.beginPath()
            ctx.strokeStyle = root.majorTickColor
            ctx.lineWidth   = tw

            for (var j = 0; j < count; j++) {
                var mtick = ruler.tickAt(j)
                if (!mtick.isMajor) continue

                var mx = vp.timeToPixel(mtick.timeMs)
                if (mx < -1 || mx > w + 1) continue

                ctx.moveTo(mx + 0.5, majLineY1)
                ctx.lineTo(mx + 0.5, majLineY2)
            }
            ctx.stroke()

            // 标签单独绘制（fillText 无法批量进 path）
            ctx.fillStyle = root.labelColor
            var lastLabelRight = -999  // 上一个标签的右边界，防重叠

            for (var k = 0; k < count; k++) {
                var ltick = ruler.tickAt(k)
                if (!ltick.isMajor || ltick.label === "") continue

                var lx = vp.timeToPixel(ltick.timeMs)
                if (lx < -100 || lx > w + 100) continue

                var labelW = ctx.measureText(ltick.label).width
                var halfW  = labelW / 2

                // 边界夹紧
                var drawX = lx
                if (lx - halfW < 2)     drawX = halfW + 2
                if (lx + halfW > w - 2) drawX = w - halfW - 2

                // 与上一个标签重叠则跳过（留 4px 间隙）
                var thisLeft = drawX - halfW
                if (thisLeft < lastLabelRight + 4) continue

                ctx.fillText(ltick.label, drawX, labelY)
                lastLabelRight = drawX + halfW
            }

            // ── 底部分隔线 ────────────────────────────────────
            if (root.showSeparator) {
                var sepY = flipped ? 0 : h - 0.5
                ctx.beginPath()
                ctx.strokeStyle = root.separatorColor
                ctx.lineWidth   = 1
                ctx.moveTo(0,   sepY)
                ctx.lineTo(w,   sepY)
                ctx.stroke()
            }
        }
    }

    // ── 触发重绘的连接 ───────────────────────────────────────

    // 刻度数据变化（视口变化后 RulerModel 自动更新并发此信号）
    Connections {
        target: root.rulerModel
        function onTicksChanged() { canvas.requestPaint() }
    }

    // 视口变化时刻度位置需要重算（即使刻度列表没变，像素坐标也变了）
    Connections {
        target: root.viewport
        function onViewChanged() { canvas.requestPaint() }
    }

    onWidthChanged:  canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    // 颜色/尺寸属性变化 → 重绘
    onMajorTickColorChanged: canvas.requestPaint()
    onMinorTickColorChanged: canvas.requestPaint()
    onLabelColorChanged:     canvas.requestPaint()
    onSeparatorColorChanged: canvas.requestPaint()
    onMajorTickHeightChanged: canvas.requestPaint()
    onMinorTickHeightChanged: canvas.requestPaint()
    onFlipChanged:           canvas.requestPaint()
}

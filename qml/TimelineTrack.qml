import QtQuick
import QuickUI.Components 1.0

/**
 * 时间轴录像轨道
 *
 * 职责：用 Canvas 把 TimelineModel 里的区间画成色块。
 * 不处理任何交互——交互由父组件的 InputHandler 处理后写入 viewport。
 *
 * 属性：
 *   viewport  — TimelineViewport，坐标换算来源（必须绑定）
 *   model     — TimelineModel，录像区间数据（必须绑定）
 *
 * 颜色：
 *   trackColor     — 轨道底色（空白部分）
 *   segmentColors  — 按 type 索引的色块颜色列表
 *                    index 0 = 普通录像，1 = 移动侦测，2 = 报警
 *                    type 超出列表时 fallback 到 index 0
 *
 * 性能说明：
 *   - onPaint 只调用 model.segmentsInRange()，O(log N + K)，不遍历全量
 *   - viewport.viewChanged 和 model.countChanged 各自 requestPaint()
 *   - Canvas renderStrategy: Canvas.Cooperative（不阻塞 GUI 线程）
 */
Item {
    id: root

    // ── 必须绑定的外部依赖 ───────────────────────────────────
    required property var viewport   // TimelineViewport
    required property var model      // TimelineModel

    // ── 外观定制 ─────────────────────────────────────────────
    property color trackColor: "#1a1a28"

    // 按 type 索引的颜色，type 超出范围时用 index 0
    property var segmentColors: [
        "#4c6ef5",   // 0: 普通录像 — 蓝紫
        "#20c997",   // 1: 移动侦测 — 青绿
        "#f03e3e",   // 2: 报警     — 红
    ]

    property real segmentHeight: height   // 色块高度，默认撑满，可缩小做分层效果
    property real segmentY: 0            // 色块顶部 Y 偏移

    // ── 圆角（0 = 无圆角，适合纯轨道；> 0 适合气泡样式）────
    property real segmentRadius: 2

    // ── Canvas ───────────────────────────────────────────────
    Canvas {
        id: canvas
        anchors.fill: parent
        renderStrategy: Canvas.Cooperative

        onPaint: {
            var ctx = getContext("2d")
            var w   = width
            var h   = height

            // 1. 清空并画底色
            ctx.clearRect(0, 0, w, h)
            ctx.fillStyle = root.trackColor
            ctx.fillRect(0, 0, w, h)

            if (!root.viewport || !root.model) return

            var vp      = root.viewport
            var vStart  = vp.viewStart
            var vEnd    = vp.viewEnd

            // 2. 取视口内区间（O(log N + K)）
            // QML 调用 C++ segmentsInRange 返回 JS 数组
            var segs = root.model.segmentsInRange(vStart, vEnd)
            if (!segs || segs.length === 0) return

            var sy     = root.segmentY
            var sh     = root.segmentHeight
            var radius = root.segmentRadius
            var colors = root.segmentColors
            var nColors = colors.length

            // 3. 批量绘制色块
            //    相同颜色的区间合并 beginPath 路径，减少 fillStyle 切换次数
            var lastColor = ""
            ctx.beginPath()

            for (var i = 0; i < segs.length; i++) {
                var seg   = segs[i]
                var type  = seg.type
                var color = (type >= 0 && type < nColors) ? colors[type] : colors[0]

                // 颜色变化时先 fill 已有路径，再开新路径
                if (color !== lastColor) {
                    if (lastColor !== "") {
                        ctx.fillStyle = lastColor
                        ctx.fill()
                    }
                    ctx.beginPath()
                    lastColor = color
                }

                // 把时间坐标转成像素，并裁剪到画布边界（避免圆角溢出）
                var x1 = Math.max(0, vp.timeToPixel(seg.startMs))
                var x2 = Math.min(w, vp.timeToPixel(seg.endMs))
                var sw = x2 - x1
                if (sw < 0.5) continue  // 太窄时跳过（亚像素区间）

                if (radius > 0 && sw > radius * 2) {
                    _roundRect(ctx, x1, sy, sw, sh, radius)
                } else {
                    ctx.rect(x1, sy, sw, sh)
                }
            }

            // flush 最后一组路径
            if (lastColor !== "") {
                ctx.fillStyle = lastColor
                ctx.fill()
            }
        }

        // 圆角矩形辅助（Canvas 2D 没有内置 roundRect 的 Qt 版本兼容写法）
        function _roundRect(ctx, x, y, w, h, r) {
            ctx.moveTo(x + r, y)
            ctx.lineTo(x + w - r, y)
            ctx.quadraticCurveTo(x + w, y,     x + w, y + r)
            ctx.lineTo(x + w, y + h - r)
            ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h)
            ctx.lineTo(x + r, y + h)
            ctx.quadraticCurveTo(x,     y + h, x,     y + h - r)
            ctx.lineTo(x,     y + r)
            ctx.quadraticCurveTo(x,     y,     x + r, y)
            ctx.closePath()
        }
    }

    // ── 触发重绘的连接 ───────────────────────────────────────

    // 视口变化（缩放 / 平移）→ 重绘
    Connections {
        target: root.viewport
        function onViewChanged() { canvas.requestPaint() }
    }

    // 数据变化（添加 / 删除区间）→ 重绘
    Connections {
        target: root.model
        function onCountChanged() { canvas.requestPaint() }
        // boundsChanged 时 viewport 范围可能更新，已由上面的 viewChanged 覆盖
    }

    // 画布尺寸变化（窗口 resize）→ 重绘
    onWidthChanged:  canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    // 颜色属性变化 → 重绘
    onTrackColorChanged:    canvas.requestPaint()
    onSegmentColorsChanged: canvas.requestPaint()
}

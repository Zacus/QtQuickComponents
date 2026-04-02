import QtQuick
import QuickUI.Components 1.0

/**
 * 时间轴录像轨道
 *
 * 使用 TimelineTrackModel 在 C++ 预计算合并矩形，
 * Canvas onPaint 直接遍历结果绘制，JS 层零换算。
 */
Item {
    id: root

    required property var viewport
    required property var model

    property color trackColor:    "#1a1a28"
    property var   segmentColors: [
        "#4c6ef5",
        "#20c997",
        "#f03e3e",
    ]
    property real segmentHeight: height
    property real segmentY:      0
    property real segmentRadius: 2
    property real mergeGapPx:    1.0

    // ── C++ 预计算层 ─────────────────────────────────────────
    TimelineTrackModel {
        id: _trackModel
        model:       root.model
        viewport:    root.viewport
        mergeGapPx:  root.mergeGapPx
        trackHeight: root.segmentHeight
        trackY:      root.segmentY
        onRectsChanged: canvas.requestPaint()
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        renderStrategy: Canvas.Cooperative

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = root.trackColor
            ctx.fillRect(0, 0, width, height)

            var colors  = root.segmentColors
            var nColors = colors.length
            var radius  = root.segmentRadius

            for (var type = 0; type < nColors; ++type) {
                var rects = _trackModel.rectsForType(type)
                if (!rects || rects.length === 0) continue

                ctx.fillStyle = colors[type]
                ctx.beginPath()

                for (var i = 0; i < rects.length; ++i) {
                    var r = rects[i]
                    if (r.width < 0.5) continue
                    if (radius > 0 && r.width > radius * 2) {
                        _roundRect(ctx, r.x, r.y, r.width, r.height, radius)
                    } else {
                        ctx.rect(r.x, r.y, r.width, r.height)
                    }
                }
                ctx.fill()
            }
        }

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

    onSegmentHeightChanged: { _trackModel.trackHeight = segmentHeight; canvas.requestPaint() }
    onSegmentYChanged:      { _trackModel.trackY = segmentY; canvas.requestPaint() }
    onWidthChanged:         canvas.requestPaint()
    onHeightChanged:        canvas.requestPaint()
    onTrackColorChanged:    canvas.requestPaint()
    onSegmentColorsChanged: canvas.requestPaint()
}

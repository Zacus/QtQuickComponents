import QtQuick
import QuickUI.Components 1.0

/**
 * 回放游标覆盖层（Playhead）
 *
 * 铺满整个轨道区域，内部子项直接用 _x 定位。
 * 不移动 Item 本身，避免子项被父 Item 裁剪。
 *
 * z 层次（从下到上）：
 *   TimelineTrack        ← 录像区间色块
 *   PlayheadOverlay      ← 游标线 + 时间标签   ← 这里
 *   TimelineInputHandler ← 轨道平移/缩放交互
 */
Item {
    id: root

    required property var viewport

    property real   currentTime:   0
    property string headPosition:  "top"
    property color  lineColor:     "#ffffff"
    property real   lineWidth:     1.5
    property color  headColor:     "#ffffff"
    property real   headSize:      10
    property bool   labelVisible:  true
    property string labelFormat:   "hh:mm:ss"
    property color  labelBg:       Qt.rgba(0, 0, 0, 0.65)
    property color  labelColor:    "#ffffff"
    property real   labelFontSize: ComponentTheme.fontSizeLabel
    property bool   autoScroll:       true
    property real   autoScrollMargin: 40

    signal seeked(real timeMs)

    readonly property bool isDragging: _headDrag.active

    // 游标像素 X（相对于本 Item 左边缘，即轨道左边缘）
    readonly property real _x: viewport ? viewport.timeToPixel(currentTime) : -1

    readonly property bool _inView: viewport
        ? (_x >= 0 && _x <= width)
        : false

    readonly property bool _headAtTop: headPosition === "top"

    // ── 竖线 ─────────────────────────────────────────────────
    Rectangle {
        id: lineItem
        visible: root._inView
        x:      root._x - root.lineWidth / 2
        y:      root._headAtTop ? root.headSize : 0
        width:  root.lineWidth
        height: root.height - root.headSize
        color:  root.lineColor
    }

    // ── 三角头 ───────────────────────────────────────────────
    Canvas {
        id: headItem
        visible: root._inView
        width:   root.headSize * 2
        height:  root.headSize
        x:       root._x - root.headSize          // 水平居中于游标线
        y:       root._headAtTop ? 0 : root.height - root.headSize

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = root.headColor
            var w = width, h = height
            ctx.beginPath()
            if (root._headAtTop) {
                ctx.moveTo(0,     0)
                ctx.lineTo(w,     0)
                ctx.lineTo(w / 2, h)
            } else {
                ctx.moveTo(w / 2, 0)
                ctx.lineTo(w,     h)
                ctx.lineTo(0,     h)
            }
            ctx.closePath()
            ctx.fill()
        }

        Component.onCompleted: requestPaint()

        // 命中区（比视觉大，方便触屏）
        Item {
            id: _hitArea
            anchors.centerIn: parent
            width:  parent.width  + 16
            height: parent.height + 16

            DragHandler {
                id: _headDrag
                target: null
                acceptedButtons: Qt.LeftButton
                grabPermissions: PointerHandler.CanTakeOverFromAnything

                onActiveChanged: {
                    cursorShape = active ? Qt.SizeHorCursor : Qt.ArrowCursor
                }

                onCentroidChanged: {
                    if (!active || !root.viewport) return

                    // centroid 相对于 _hitArea，_hitArea 中心在 headItem 中心，
                    // headItem.x = _x - headSize，headItem 中心 x = _x
                    // 所以轨道绝对 X = centroid.x + headItem.x + headItem.width/2
                    var absX = centroid.position.x
                                + headItem.x + headItem.width / 2
                    var t = root.viewport.pixelToTime(absX)
                    var tClamped = Math.max(
                        root.viewport.totalStart,
                        Math.min(root.viewport.totalEnd, t)
                    )

                    root.currentTime = tClamped
                    root.seeked(tClamped)

                    if (root.autoScroll) {
                        var px = root.viewport.timeToPixel(tClamped)
                        var margin = root.autoScrollMargin
                        var vw = root.viewport.viewWidth
                        if (px < margin)
                            root.viewport.panBy(px - margin)
                        else if (px > vw - margin)
                            root.viewport.panBy(px - (vw - margin))
                    }
                }

                cursorShape: Qt.SizeHorCursor
            }
        }
    }

    // ── 时间标签 ─────────────────────────────────────────────
    Item {
        id: timeLabelContainer
        visible: root.labelVisible && root._inView
        y: root._headAtTop ? 0 : root.height - root.headSize - labelBox.height - 2

        // 理想位置居中于游标线，夹在左右边界内
        x: {
            var ideal = root._x - labelBox.width / 2
            return Math.max(2, Math.min(root.width - labelBox.width - 2, ideal))
        }

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
                    var dt = new Date(root.currentTime)
                    return Qt.formatTime(dt, root.labelFormat)
                }
                color:          root.labelColor
                font.pixelSize: root.labelFontSize
                font.family:    ComponentTheme.fontFamily
            }
        }
    }

    onHeadColorChanged:    headItem.requestPaint()
    onHeadPositionChanged: headItem.requestPaint()
    onHeadSizeChanged:     headItem.requestPaint()
}

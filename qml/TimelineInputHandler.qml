import QtQuick
import QuickUI.Components 1.0

/**
 * 时间轴交互层
 *
 * 透明覆盖层，拦截用户输入并翻译成 viewport 操作。
 * 自身不绘制任何内容，叠放在 TimelineTrack / TimelineRuler 上方。
 *
 * ┌─────────────────────────────────────────┐
 * │  TimelineInputHandler  (z 最高)          │
 * │  ┌───────────────────────────────────┐  │
 * │  │  TimelineRuler                    │  │
 * │  ├───────────────────────────────────┤  │
 * │  │  TimelineTrack                    │  │
 * │  └───────────────────────────────────┘  │
 * └─────────────────────────────────────────┘
 *
 * 支持的交互：
 *   滚轮          — 以鼠标位置为锚点缩放（Ctrl 可选，默认不需要）
 *   左键拖拽      — 平移（光标变抓手）
 *   左键单击      — 设置 currentTime（游标跳转）
 *   左键双击      — fitAll()（回到全局视图）
 *   Pinch（触屏） — 以手势中心为锚点缩放
 *   右键 / 中键   — 不处理，透传给父组件
 *
 * 属性：
 *   viewport        — TimelineViewport，写入目标（必须绑定）
 *   currentTime     — 当前游标时间（ms），双向，由点击和外部均可驱动
 *   zoomFactor      — 每格滚轮的缩放倍率（默认 1.12）
 *   requireCtrl     — true 时滚轮缩放需按住 Ctrl（默认 false）
 *   dragThreshold   — 区分点击/拖拽的像素阈值（默认 4px）
 *
 * 信号：
 *   seeked(timeMs)  — 用户点击/拖拽游标后发出，携带目标时间（ms）
 *   zoomChanged()   — 缩放发生后发出（用于通知外部 UI 更新缩放显示）
 */
Item {
    id: root

    // ── 必须绑定 ─────────────────────────────────────────────
    required property var viewport    // TimelineViewport

    // ── 游标时间（双向绑定，外部可设置，内部点击也会更新）──
    property real currentTime: 0      // ms，用 real 而不是 int 保留亚毫秒精度

    // ── 行为调节 ─────────────────────────────────────────────
    property real zoomFactor:   1.12   // 每格滚轮放大倍率
    property bool requireCtrl:  false  // 是否要求按 Ctrl 才触发滚轮缩放
    property real dragThreshold: 4     // 像素：超过此距离才算拖拽（不触发点击）

    // ── 信号 ─────────────────────────────────────────────────
    signal seeked(real timeMs)
    signal zoomChanged()

    // ── 只读状态（供父组件显示状态指示器）──────────────────
    readonly property bool isDragging: _drag.active

    // ══════════════════════════════════════════════════════════
    // 内部状态
    // ══════════════════════════════════════════════════════════
    // 拖拽开始时记录的视口起点，用于计算平移增量
    property real _dragStartViewStart: 0
    // 拖拽起始像素位置
    property real _dragStartX: 0
    // 是否已经超过拖拽阈值（区分点击和拖拽）
    property bool _didDrag: false

    // ══════════════════════════════════════════════════════════
    // 滚轮缩放
    // ══════════════════════════════════════════════════════════
    WheelHandler {
        id: _wheel
        target: null          // 不移动 Item，手动处理
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        // TouchPad 的双指滑动也走 WheelHandler，pixelDelta 非零时用 pixelDelta
        onWheel: function(event) {
            if (root.requireCtrl && !(event.modifiers & Qt.ControlModifier)) {
                event.accepted = false
                return
            }

            if (!root.viewport) { event.accepted = false; return }

            // 区分触控板像素增量和鼠标滚轮格增量
            var delta
            if (event.pixelDelta.y !== 0) {
                // 触控板：pixelDelta 单位是像素，通常 ±3~15
                // 把每 120px 累计等价为一格鼠标滚轮
                delta = event.pixelDelta.y / 120.0
            } else {
                // 鼠标滚轮：angleDelta 单位是 1/8 度，一格 = 120 (= 15°)
                delta = event.angleDelta.y / 120.0
            }

            if (Math.abs(delta) < 0.01) return

            var factor = delta > 0
                ? Math.pow(root.zoomFactor, delta)      // 向上 → 放大
                : Math.pow(1.0 / root.zoomFactor, -delta) // 向下 → 缩小

            root.viewport.zoomAt(event.x, factor)
            root.zoomChanged()
            event.accepted = true
        }
    }

    // ══════════════════════════════════════════════════════════
    // 拖拽平移
    // ══════════════════════════════════════════════════════════
    DragHandler {
        id: _drag
        target: null                    // 不移动 Item
        acceptedButtons: Qt.LeftButton
        dragThreshold: 0                // 我们自己判断阈值

        onActiveChanged: {
            if (active) {
                // 拖拽开始：记录快照
                root._dragStartX = centroid.position.x
                root._dragStartViewStart = root.viewport ? root.viewport.viewStart : 0
                root._didDrag = false
                cursorShape = Qt.ClosedHandCursor
            } else {
                cursorShape = Qt.ArrowCursor
                // 如果没有真正拖拽（delta < threshold），当作点击处理
                if (!root._didDrag && root.viewport) {
                    var t = root.viewport.pixelToTime(centroid.position.x)
                    root.currentTime = t
                    root.seeked(t)
                }
            }
        }

        onCentroidChanged: {
            if (!active || !root.viewport) return

            var dx = centroid.position.x - root._dragStartX

            // 超过阈值才真正开始平移
            if (!root._didDrag) {
                if (Math.abs(dx) < root.dragThreshold) return
                root._didDrag = true
            }

            // 平移：锚定起始 viewStart，用像素差推算时间差
            // 向右拖 → dx > 0 → 时间向过去方向移动 → viewStart 减小
            var deltaMs = root.viewport.pixelsToDuration(dx)
            var newStart = root._dragStartViewStart - deltaMs
            var span = root.viewport.viewEnd - root.viewport.viewStart
            // 直接写 viewStart 触发 applyView → 边界保护自动生效
            root.viewport.setViewStart(newStart)
        }

        cursorShape: Qt.OpenHandCursor
    }

    // ══════════════════════════════════════════════════════════
    // 单击设置游标 / 双击 fitAll
    // ══════════════════════════════════════════════════════════
    TapHandler {
        id: _tap
        acceptedButtons: Qt.LeftButton
        // 与 DragHandler 共存：DragHandler 负责超阈值后的平移，
        // TapHandler 只在没有发生拖拽时才应该触发。
        // Qt 6 的 gesturePolicy: TapHandler.WithinBounds 可以做到：
        // 如果手指/鼠标移出了组件边界，tap 被取消。
        gesturePolicy: TapHandler.WithinBounds

        onSingleTapped: function(eventPoint, button) {
            // DragHandler 已经在 onActiveChanged(false) 里处理了无拖拽的点击，
            // 这里只处理 DragHandler 没有激活的情况（极短点击）
            if (root._didDrag) return
            if (!root.viewport) return
            var t = root.viewport.pixelToTime(eventPoint.position.x)
            root.currentTime = t
            root.seeked(t)
        }

        onDoubleTapped: function(eventPoint, button) {
            if (!root.viewport) return
            root.viewport.fitAll()
            root.zoomChanged()
        }
    }

    // ══════════════════════════════════════════════════════════
    // 触屏 Pinch 缩放
    // ══════════════════════════════════════════════════════════
    PinchHandler {
        id: _pinch
        target: null
        acceptedDevices: PointerDevice.TouchScreen

        // 记录 pinch 开始时的视口状态
        property real _startScale:     1.0
        property real _startViewStart: 0
        property real _startViewSpan:  0
        property real _startCenterX:   0

        onActiveChanged: {
            if (active && root.viewport) {
                _startScale      = 1.0
                _startViewStart  = root.viewport.viewStart
                _startViewSpan   = root.viewport.viewSpan
                _startCenterX    = centroid.position.x
            }
        }

        onScaleChanged: {
            if (!active || !root.viewport) return

            // pinch 给出的 scale 是相对于手势开始时的累计倍率
            // 我们用"开始时的视口状态 + 当前 scale"重新计算，
            // 避免每帧叠乘导致浮点漂移
            var scale = activeScale   // Qt 6.5+ 属性，累计缩放倍率
            if (scale <= 0) return

            var anchorX   = centroid.position.x
            var newSpan   = _startViewSpan / scale

            // 锚点时间保持不变
            // anchorTime = _startViewStart + _startCenterX / (_startViewWidth / _startViewSpan)
            // 但我们没存 _startViewWidth，用当前 width 近似（pinch 过程中 width 不变）
            var ppm_old = (root.width > 0 && _startViewSpan > 0)
                          ? root.width / _startViewSpan : 1
            var anchorTime = _startViewStart + _startCenterX / ppm_old

            var ppm_new = root.width / newSpan
            var newStart = anchorTime - anchorX / ppm_new

            root.viewport.fitRange(newStart, newStart + newSpan)
            root.zoomChanged()
        }
    }

    // ══════════════════════════════════════════════════════════
    // 键盘快捷键
    // ══════════════════════════════════════════════════════════
    Keys.onPressed: function(event) {
        if (!root.viewport) return
        var step = root.viewport.viewSpan * 0.1   // 每次平移 10% 视口宽度

        switch (event.key) {
        case Qt.Key_Left:
            root.viewport.panBy(-root.width * 0.1)
            event.accepted = true
            break
        case Qt.Key_Right:
            root.viewport.panBy(root.width * 0.1)
            event.accepted = true
            break
        case Qt.Key_Plus:
        case Qt.Key_Equal:
            root.viewport.zoomAt(root.width / 2, root.zoomFactor)
            root.zoomChanged()
            event.accepted = true
            break
        case Qt.Key_Minus:
            root.viewport.zoomAt(root.width / 2, 1.0 / root.zoomFactor)
            root.zoomChanged()
            event.accepted = true
            break
        case Qt.Key_Home:
            root.viewport.fitAll()
            root.zoomChanged()
            event.accepted = true
            break
        default:
            break
        }
    }

    // 允许接收键盘事件（需要 focus）
    focus: true
    // 点击时自动获取 focus
    TapHandler {
        acceptedButtons: Qt.AllButtons
        onTapped: root.forceActiveFocus()
    }
}

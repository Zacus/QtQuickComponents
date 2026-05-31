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
    property real zoomFactor:     1.12  // 每格滚轮放大倍率
    property bool requireCtrl:    false // 是否要求按 Ctrl 才触发滚轮缩放
    property real dragThreshold:  4     // 像素：超过此距离才算拖拽（不触发点击）

    /**
     * 拖拽灵敏度（0.1 ~ 1.0）。
     * 1.0 = 1px 对应当前视口密度下的完整时间偏移（原始速度，视口越宽越快）
     * 0.3 = 拖拽速度降低到原始的 30%，精细操控更容易
     * 推荐：磁带模型下设 0.3，视频编辑模型下设 1.0
     */
    property real dragSensitivity: 0.3

    /**
     * 跟随模式（与 TimelineView.followMode 保持同步）：
     *   TimelineEnums.FollowCenter → 磁带模型：拖拽 = 连续 seek，游标跟着走
     *   其他                       → 视频编辑模型：拖拽 = 平移视口
     */
    property int followMode: TimelineEnums.FollowEdge

    // ── 信号 ─────────────────────────────────────────────────
    signal seeked(real timeMs)
    signal zoomChanged()
    signal playRequested(real timeMs)   // 双击：请求从指定时间开始播放

    // ── 只读状态（供父组件显示状态指示器）──────────────────
    readonly property bool isDragging: _drag.active

    // ── 内部状态 ──────────────────────────────────────────────
    property real _dragStartViewStart: 0   // 视口编辑模型：拖拽起点的 viewStart
    property real _dragStartX:         0   // 拖拽起点像素（用于阈值判断和点击检测）
    property real _lastDragX:          0   // 磁带模型：上一帧的像素位置（逐帧增量用）
    property bool _didDrag:            false

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

            // angleDelta 优先（鼠标滚轮和触控板都有 angleDelta）。
            // macOS 上鼠标滚轮同时产生 angleDelta 和 pixelDelta，
            // 若优先用 pixelDelta 会得到极小的 delta 值导致 span 迅速压缩到最小值。
            // pixelDelta 仅在 angleDelta 为零时使用（纯像素滚动设备）。
            var delta
            if (event.angleDelta.y !== 0) {
                // 鼠标滚轮 / 触控板：angleDelta 单位是 1/8 度，一格 = 120
                delta = event.angleDelta.y / 120.0
            } else if (event.pixelDelta.y !== 0) {
                // 纯像素滚动设备（部分触控板）：pixelDelta，以 60px 为一格
                delta = event.pixelDelta.y / 60.0
            } else {
                return
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
    // 拖拽：磁带模型 = 连续 seek，视频编辑模型 = 平移视口
    // ══════════════════════════════════════════════════════════
    DragHandler {
        id: _drag
        target: null
        acceptedButtons: Qt.LeftButton
        dragThreshold: 0   // 阈值由内部 _didDrag 自己管

        onActiveChanged: {
            if (active) {
                root._dragStartX         = centroid.position.x
                root._lastDragX          = centroid.position.x
                root._dragStartViewStart = root.viewport ? root.viewport.viewStart : 0
                root._didDrag = false
                cursorShape = Qt.ClosedHandCursor
            } else {
                cursorShape = Qt.ArrowCursor
                // 没有超过阈值 → 当作单击处理
                if (!root._didDrag && root.viewport) {
                    var t = root.viewport.pixelToTime(centroid.position.x)
                    root.currentTime = t
                    root.seeked(t)
                }
            }
        }

        onCentroidChanged: {
            if (!active || !root.viewport) return

            var curX = centroid.position.x
            var dx   = curX - root._dragStartX   // 相对起点的总偏移（阈值判断用）

            if (!root._didDrag) {
                if (Math.abs(dx) < root.dragThreshold) return
                root._didDrag   = true
                root._lastDragX = curX   // 阈值刚过时重置增量起点，避免起步跳动
            }

            if (root.followMode === TimelineEnums.FollowCenter) {
                // ── 磁带模型：逐帧增量 seek ───────────────────
                // 用"这一帧相对上一帧的偏移"而不是"相对拖拽起点的偏移"：
                //   - 方向正确：向左 frameDx<0 → 时间前进，向右 frameDx>0 → 时间后退
                //   - 速度一致：视口移动不影响换算，pixelsPerMs 保持一帧内稳定
                //   - 可换向：松开再往另一个方向拖，frameDx 立即变号
                var frameDx  = curX - root._lastDragX
                root._lastDragX = curX

                // 向左拖（frameDx < 0）→ 时间增大（往未来走）
                // 向右拖（frameDx > 0）→ 时间减小（往过去走）
                var deltaMs = root.viewport.pixelsToDuration(-frameDx * root.dragSensitivity)
                var seekTime = root.currentTime + deltaMs
                seekTime = Math.max(root.viewport.totalStart,
                           Math.min(root.viewport.totalEnd, seekTime))
                root.currentTime = seekTime
                root.seeked(seekTime)
            } else {
                // ── 视频编辑模型：锚定起点的总偏移平移视口 ───
                var scaledDx = dx * root.dragSensitivity
                var deltaMs2 = root.viewport.pixelsToDuration(scaledDx)
                root.viewport.viewStart = root._dragStartViewStart - deltaMs2
            }
        }

        cursorShape: Qt.OpenHandCursor
    }

    // ══════════════════════════════════════════════════════════
    // 单击设置游标 / 双击在该位置播放
    // ══════════════════════════════════════════════════════════
    TapHandler {
        id: _tap
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.WithinBounds

        onSingleTapped: function(eventPoint, button) {
            if (root._didDrag) return
            if (!root.viewport) return
            var t = root.viewport.pixelToTime(eventPoint.position.x)
            root.currentTime = t
            root.seeked(t)
        }

        onDoubleTapped: function(eventPoint, button) {
            if (!root.viewport) return
            // 双击：跳转到双击位置并开始播放（发 seeked 信号，由外部启动播放）
            var t = root.viewport.pixelToTime(eventPoint.position.x)
            root.currentTime = t
            root.seeked(t)
            root.playRequested(t)
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

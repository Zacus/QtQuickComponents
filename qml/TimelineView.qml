import QtQuick
import QuickUI.Components 1.0

/**
 * 时间轴完整组件（根组件）
 *
 * ═══════════════════════════════════════════════════════
 * 最小用法：
 * ═══════════════════════════════════════════════════════
 *
 *   TimelineView {
 *       width:       parent.width
 *       height:      120
 *       model:       myTimelineModel
 *       currentTime: player.positionMs
 *       onSeeked:    function(t) { player.seek(t) }
 *   }
 *
 * ═══════════════════════════════════════════════════════
 * followMode 跟随模式：
 * ═══════════════════════════════════════════════════════
 *
 *   "none"   — 不自动滚动（默认），用户手动平移
 *   "edge"   — 游标到达视口边缘 followMargin 内时，视口平移
 *              使游标保持在 followMargin 处（类似视频编辑软件）
 *   "center" — 视口始终以游标为中心滚动（类似音频软件）
 *
 *   用户手动拖拽轨道时自动暂停跟随，松手后恢复。
 *
 * ═══════════════════════════════════════════════════════
 * currentTime 数据流（单向，无双向绑定陷阱）：
 * ═══════════════════════════════════════════════════════
 *
 *   player.positionMs ──→ currentTime ──→ PlayheadOverlay 位置
 *   用户操作 → onSeeked(t) → 外部 player.seek(t) → currentTime 更新
 */
Item {
    id: root

    // ══════════════════════════════════════════════════════
    // 对外接口 — 数据
    // ══════════════════════════════════════════════════════

    property var  model:       null
    property real currentTime: 0

    // ══════════════════════════════════════════════════════
    // 对外接口 — 布局
    // ══════════════════════════════════════════════════════

    property real rulerHeight: 28

    // ══════════════════════════════════════════════════════
    // 对外接口 — 外观
    // ══════════════════════════════════════════════════════

    property color  trackColor:    "#1a1a28"
    property var    segmentColors: ["#4c6ef5", "#20c997", "#f03e3e"]
    property real   segmentRadius: 2

    property color  rulerBg:             "transparent"
    property color  rulerMajorColor:     ComponentTheme.textSecondary
    property color  rulerMinorColor:     Qt.rgba(
                                             ComponentTheme.textSecondary.r,
                                             ComponentTheme.textSecondary.g,
                                             ComponentTheme.textSecondary.b, 0.4)
    property color  rulerLabelColor:     ComponentTheme.textSecondary
    property color  rulerSeparatorColor: ComponentTheme.separator
    property string rulerLabelFont:      "11px sans-serif"

    property color  playheadColor:        "#ffffff"
    property real   playheadLineWidth:    1.5
    property real   playheadHeadSize:     10
    property bool   playheadLabelVisible: true
    property string playheadLabelFormat:  "hh:mm:ss"
    property color  playheadLabelBg:      Qt.rgba(0, 0, 0, 0.65)
    property color  playheadLabelColor:   "#ffffff"

    // ══════════════════════════════════════════════════════
    // 对外接口 — 跟随模式
    // ══════════════════════════════════════════════════════

    /**
     * 游标跟随模式：
     *   TimelineEnums.FollowNone   — 不自动滚动
     *   TimelineEnums.FollowEdge   — 游标到达视口边缘时平移（推荐默认）
     *   TimelineEnums.FollowCenter — 视口始终居中于游标
     */
    property int followMode: TimelineEnums.FollowEdge

    /**
     * edge 模式下，触发平移的边缘宽度（px）。
     * 游标距视口右边缘 < followMargin 时开始推进。
     */
    property real followMargin: 80

    // ══════════════════════════════════════════════════════
    // 对外接口 — 交互行为
    // ══════════════════════════════════════════════════════

    property real zoomFactor:      1.12
    property bool requireCtrl:     false
    property real autoScrollMargin: 40

    // ══════════════════════════════════════════════════════
    // 对外接口 — 只读状态
    // ══════════════════════════════════════════════════════

    readonly property bool isPanning: _input.isDragging
    readonly property bool isSeeking: _playhead.isDragging
    readonly property alias viewport: _viewport

    // ══════════════════════════════════════════════════════
    // 对外接口 — 信号
    // ══════════════════════════════════════════════════════

    signal seeked(real timeMs)
    signal zoomChanged()
    /**
     * 用户双击时间轴发出，携带双击位置对应的时间（ms）。
     * 外部在此处理器中启动播放：
     *   onPlayRequested: function(t) { player.seek(t); player.play() }
     */
    signal playRequested(real timeMs)

    // ══════════════════════════════════════════════════════
    // 对外接口 — 方法
    // ══════════════════════════════════════════════════════

    function fitAll()                 { _viewport.fitAll() }
    function fitRange(startMs, endMs) { _viewport.fitRange(startMs, endMs) }
    function centerOnTime(timeMs)     { _viewport.centerOnTime(timeMs) }
    function zoom(factor)             { _viewport.zoomAt(_trackArea.width / 2, factor) }

    // ══════════════════════════════════════════════════════
    // 内部对象
    // ══════════════════════════════════════════════════════

    TimelineViewport {
        id: _viewport
        viewWidth:  _trackArea.width
        totalStart: root.model ? root.model.totalStart : 0
        totalEnd:   root.model ? root.model.totalEnd   : 0

        onTotalRangeChanged: {
            if (totalStart < totalEnd && _viewport._firstLoad) {
                _viewport._firstLoad = false
                Qt.callLater(_viewport.fitAll)
            }
        }

        property bool _firstLoad: true
    }

    RulerModel {
        id: _ruler
        viewport: _viewport
    }

    // ══════════════════════════════════════════════════════
    // 跟随逻辑
    // ══════════════════════════════════════════════════════

    // 用户正在手动拖拽时暂停跟随，避免两个逻辑打架
    readonly property bool _userPanning: _input.isDragging || _playhead.isDragging

    // currentTime 变化时执行跟随
    onCurrentTimeChanged: {
        if (root.followMode === TimelineEnums.FollowNone) return
        if (root._userPanning) return
        if (!_viewport || _viewport.viewWidth <= 0) return

        var px = _viewport.timeToPixel(root.currentTime)
        var vw = _viewport.viewWidth

        if (root.followMode === TimelineEnums.FollowCenter) {
            // 居中模式：视口始终以游标为中心
            _viewport.centerOnTime(root.currentTime)

        } else if (root.followMode === TimelineEnums.FollowEdge) {
            // 边缘模式：游标超出右边缘触发推进，超出左边缘触发回退
            // 右边缘：游标 > vw - followMargin → 把游标推到 followMargin 处
            if (px > vw - root.followMargin) {
                var targetStart = root.currentTime
                    - (vw - root.followMargin) / _viewport.pixelsPerMs
                _viewport.viewStart = targetStart
            }
            // 左边缘：游标 < followMargin → 把游标推到 vw - followMargin 处
            else if (px < root.followMargin) {
                var targetStart2 = root.currentTime
                    - root.followMargin / _viewport.pixelsPerMs
                _viewport.viewStart = targetStart2
            }
        }
    }

    // ══════════════════════════════════════════════════════
    // 背景
    // ══════════════════════════════════════════════════════
    Rectangle {
        anchors.fill: parent
        color: root.trackColor
        z: -1
    }

    // ══════════════════════════════════════════════════════
    // 布局
    // ══════════════════════════════════════════════════════

    TimelineRuler {
        id: _rulerItem
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: root.rulerHeight

        viewport:         _viewport
        rulerModel:       _ruler
        backgroundColor:  root.rulerBg
        majorTickColor:   root.rulerMajorColor
        minorTickColor:   root.rulerMinorColor
        labelColor:       root.rulerLabelColor
        separatorColor:   root.rulerSeparatorColor
        labelFont:        root.rulerLabelFont
    }

    Item {
        id: _trackArea
        anchors {
            top:    _rulerItem.bottom
            left:   parent.left
            right:  parent.right
            bottom: parent.bottom
        }

        TimelineTrack {
            id: _track
            anchors.fill: parent
            z: 0
            viewport:      _viewport
            model:         root.model
            trackColor:    root.trackColor
            segmentColors: root.segmentColors
            segmentRadius: root.segmentRadius
        }

        PlayheadOverlay {
            id: _playhead
            anchors.fill: parent
            z: 1
            viewport:         _viewport
            currentTime:      root.currentTime
            lineColor:        root.playheadColor
            lineWidth:        root.playheadLineWidth
            headColor:        root.playheadColor
            headSize:         root.playheadHeadSize
            labelVisible:     root.playheadLabelVisible
            labelFormat:      root.playheadLabelFormat
            labelBg:          root.playheadLabelBg
            labelColor:       root.playheadLabelColor
            autoScrollMargin: root.autoScrollMargin
            onSeeked: function(t) { root.seeked(t) }
        }

        TimelineInputHandler {
            id: _input
            anchors.fill: parent
            z: 2
            viewport:    _viewport
            currentTime: root.currentTime
            zoomFactor:  root.zoomFactor
            requireCtrl: root.requireCtrl
            onSeeked:         function(t) { root.seeked(t) }
            onZoomChanged:    root.zoomChanged()
            onPlayRequested:  function(t) { root.playRequested(t) }
        }
    }
}

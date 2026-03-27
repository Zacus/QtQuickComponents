import QtQuick
import QuickUI.Components 1.0

/**
 * 时间轴完整组件（根组件）
 *
 * 把所有子件组合在一起，对外只暴露调用方真正需要的属性。
 * 内部的 TimelineViewport、RulerModel 由本组件自行创建和管理。
 *
 * ═══════════════════════════════════════════════════════
 * 最小用法：
 * ═══════════════════════════════════════════════════════
 *
 *   TimelineView {
 *       width:       parent.width
 *       height:      120
 *       model:       myTimelineModel       // TimelineModel（C++ 侧创建）
 *       currentTime: player.positionMs     // 播放器位置 → 驱动游标
 *       onSeeked:    function(t) { player.seek(t) }
 *   }
 *
 * ═══════════════════════════════════════════════════════
 * 完整用法（覆盖外观 + 高级控制）：
 * ═══════════════════════════════════════════════════════
 *
 *   TimelineView {
 *       width:  parent.width
 *       height: 140
 *       model:  myTimelineModel
 *
 *       currentTime:  player.positionMs
 *       onSeeked:     function(t) { player.seek(t) }
 *
 *       rulerHeight:         28
 *       trackColor:          "#12121e"
 *       segmentColors:       ["#5c7cfa", "#20c997", "#f03e3e"]
 *       playheadColor:       "#ffffff"
 *       playheadLabelFormat: "hh:mm:ss.zzz"
 *       zoomFactor:          1.15
 *       requireCtrl:         true
 *
 *       // 直接访问内部视口（高级用法）
 *       Component.onCompleted: viewport.fitAll()
 *   }
 *
 * ═══════════════════════════════════════════════════════
 * currentTime 数据流（单向，无双向绑定陷阱）：
 * ═══════════════════════════════════════════════════════
 *
 *   外部播放器         TimelineView          子件
 *   player.positionMs ──→ currentTime ──→  PlayheadOverlay._x
 *                                      └→  InputHandler（只读显示）
 *
 *   用户拖游标/点轨道 → onSeeked(t) → 外部调用 player.seek(t)
 *                                  → 外部更新 player.positionMs
 *                                  → currentTime 随之更新（单向）
 *
 *   不在内部 onSeeked 里写 root.currentTime = t，
 *   保持属性绑定链完整，由外部播放器决定是否接受 seek。
 *
 * ═══════════════════════════════════════════════════════
 * 内部层叠顺序（z 从小到大）：
 * ═══════════════════════════════════════════════════════
 *
 *   ┌────────────────────────────────────────────┐
 *   │ background Rectangle              z = -1   │
 *   ├────────────────────────────────────────────┤
 *   │ TimelineRuler   (顶部，rulerHeight)  z = 0 │
 *   ├────────────────────────────────────────────┤  ← _trackArea
 *   │ TimelineTrack                       z = 0  │
 *   │ PlayheadOverlay                     z = 1  │
 *   │ TimelineInputHandler                z = 2  │
 *   └────────────────────────────────────────────┘
 */
Item {
    id: root

    // ══════════════════════════════════════════════════════
    // 对外接口 — 数据
    // ══════════════════════════════════════════════════════

    /** TimelineModel（C++ 侧创建并注入） */
    property var model: null

    /**
     * 当前游标时间（ms since epoch）。
     * 通常由外部播放器单向驱动：currentTime: player.positionMs
     * 用户操作时只发 seeked 信号，不在内部写此属性，保持绑定链完整。
     */
    property real currentTime: 0

    // ══════════════════════════════════════════════════════
    // 对外接口 — 布局
    // ══════════════════════════════════════════════════════

    /** 刻度尺高度（px） */
    property real rulerHeight: 28

    // ══════════════════════════════════════════════════════
    // 对外接口 — 外观
    // ══════════════════════════════════════════════════════

    // 轨道 ─────────────────────────────────────────────────
    property color trackColor:    "#1a1a28"
    property var   segmentColors: ["#4c6ef5", "#20c997", "#f03e3e"]
    property real  segmentRadius: 2

    // 刻度尺 ───────────────────────────────────────────────
    property color  rulerBg:           "transparent"
    property color  rulerMajorColor:   ComponentTheme.textSecondary
    property color  rulerMinorColor:   Qt.rgba(
                                           ComponentTheme.textSecondary.r,
                                           ComponentTheme.textSecondary.g,
                                           ComponentTheme.textSecondary.b, 0.4)
    property color  rulerLabelColor:   ComponentTheme.textSecondary
    property color  rulerSeparatorColor: ComponentTheme.separator
    property string rulerLabelFont:    "11px sans-serif"

    // 游标 ─────────────────────────────────────────────────
    property color  playheadColor:        "#ffffff"
    property real   playheadLineWidth:    1.5
    property real   playheadHeadSize:     10
    property bool   playheadLabelVisible: true
    property string playheadLabelFormat:  "hh:mm:ss"
    property color  playheadLabelBg:      Qt.rgba(0, 0, 0, 0.65)
    property color  playheadLabelColor:   "#ffffff"

    // ══════════════════════════════════════════════════════
    // 对外接口 — 交互行为
    // ══════════════════════════════════════════════════════

    /** 每格滚轮的缩放倍率（默认 1.12） */
    property real zoomFactor:   1.12

    /** true 时滚轮缩放需按住 Ctrl */
    property bool requireCtrl:  false

    /** 自动滚动触发边缘（px），拖游标到边缘内触发视口平移 */
    property real autoScrollMargin: 40

    // ══════════════════════════════════════════════════════
    // 对外接口 — 只读状态
    // ══════════════════════════════════════════════════════

    /** 是否正在拖拽轨道（平移视口中） */
    readonly property bool isPanning: _input.isDragging

    /** 是否正在拖拽游标 */
    readonly property bool isSeeking: _playhead.isDragging

    /**
     * 内部视口（高级用法：直接调用 viewport.fitAll() 等）
     * 只读引用，请勿替换。
     */
    readonly property alias viewport: _viewport

    // ══════════════════════════════════════════════════════
    // 对外接口 — 信号
    // ══════════════════════════════════════════════════════

    /**
     * 用户在时间轴上点击或拖拽游标时发出，携带目标时间（ms）。
     * 外部播放器在此处理器中调用 seek 并更新 positionMs，
     * positionMs 绑定到 currentTime 后游标自动跟随——无需在这里写 currentTime。
     */
    signal seeked(real timeMs)

    /** 缩放级别变化后发出（可用于刷新外部缩放指示器） */
    signal zoomChanged()

    // ══════════════════════════════════════════════════════
    // 对外接口 — 方法
    // ══════════════════════════════════════════════════════

    /** 将视口适配到数据总范围 */
    function fitAll()               { _viewport.fitAll() }

    /** 将视口适配到指定时间范围（ms） */
    function fitRange(startMs, endMs) { _viewport.fitRange(startMs, endMs) }

    /** 将指定时间居中显示，不改变缩放级别 */
    function centerOnTime(timeMs)   { _viewport.centerOnTime(timeMs) }

    /** 以视口中心为锚点缩放，factor > 1 放大，< 1 缩小 */
    function zoom(factor)           { _viewport.zoomAt(_trackArea.width / 2, factor) }

    // ══════════════════════════════════════════════════════
    // 内部对象
    // ══════════════════════════════════════════════════════

    TimelineViewport {
        id: _viewport

        // 画布宽度绑定到轨道区域实际宽度
        viewWidth: _trackArea.width

        // 数据总范围随 model 更新
        totalStart: root.model ? root.model.totalStart : 0
        totalEnd:   root.model ? root.model.totalEnd   : 0

        // 首次有效数据到达时自动 fitAll，之后交给用户控制
        onTotalRangeChanged: {
            if (totalStart < totalEnd && _viewport._firstLoad) {
                _viewport._firstLoad = false
                // callLater 确保 viewWidth 已由布局赋值
                Qt.callLater(_viewport.fitAll)
            }
        }

        // 内部标志：只自动 fitAll 一次
        property bool _firstLoad: true
    }

    RulerModel {
        id: _ruler
        viewport: _viewport
    }

    // ══════════════════════════════════════════════════════
    // 背景（防止透明区域漏出父级内容）
    // ══════════════════════════════════════════════════════
    Rectangle {
        anchors.fill: parent
        color: root.trackColor
        z: -1
    }

    // ══════════════════════════════════════════════════════
    // 布局
    // ══════════════════════════════════════════════════════

    // ── 刻度尺（顶部，固定高度）──────────────────────────
    TimelineRuler {
        id: _rulerItem
        anchors {
            top:   parent.top
            left:  parent.left
            right: parent.right
        }
        height: root.rulerHeight

        viewport:   _viewport
        rulerModel: _ruler

        backgroundColor:  root.rulerBg
        majorTickColor:   root.rulerMajorColor
        minorTickColor:   root.rulerMinorColor
        labelColor:       root.rulerLabelColor
        separatorColor:   root.rulerSeparatorColor
        labelFont:        root.rulerLabelFont
    }

    // ── 轨道区域（刻度尺下方，占剩余全部高度）────────────
    Item {
        id: _trackArea
        anchors {
            top:    _rulerItem.bottom
            left:   parent.left
            right:  parent.right
            bottom: parent.bottom
        }

        // ── 录像区间轨道（最底层，z=0）───────────────────
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

        // ── 游标覆盖层（z=1）─────────────────────────────
        PlayheadOverlay {
            id: _playhead
            anchors.fill: parent
            z: 1

            viewport:    _viewport
            // 单向：外部 currentTime → 游标位置。拖拽结果通过 onSeeked 传出。
            currentTime: root.currentTime

            lineColor:        root.playheadColor
            lineWidth:        root.playheadLineWidth
            headColor:        root.playheadColor
            headSize:         root.playheadHeadSize
            labelVisible:     root.playheadLabelVisible
            labelFormat:      root.playheadLabelFormat
            labelBg:          root.playheadLabelBg
            labelColor:       root.playheadLabelColor
            autoScrollMargin: root.autoScrollMargin

            // 游标拖拽 → 告知外部，外部决定是否接受（由外部更新 currentTime）
            onSeeked: function(t) { root.seeked(t) }
        }

        // ── 交互层（最上层，透明，z=2）───────────────────
        TimelineInputHandler {
            id: _input
            anchors.fill: parent
            z: 2

            viewport:    _viewport
            // InputHandler 的 currentTime 仅供其内部点击时读取初始位置用，
            // 保持与 root.currentTime 同步即可，不需要双向绑定
            currentTime: root.currentTime

            zoomFactor:  root.zoomFactor
            requireCtrl: root.requireCtrl

            // 轨道点击/拖拽完成 → 告知外部
            onSeeked:      function(t) { root.seeked(t) }
            onZoomChanged: root.zoomChanged()
        }
    }
}

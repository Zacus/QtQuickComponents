import QtQuick
import QtQuick.Controls.Basic
import QuickUI.Components 1.0

// 通用按钮，支持三种外观变体：
//   variant: "filled"  — 强调色实底（默认）
//   variant: "outline" — 透明底 + 强调色描边
//   variant: "ghost"   — 纯透明，仅显示文字，适合次要操作
//
// 状态：
//   enabled: false — 禁用，阻断交互并降低透明度
//   loading: true  — 显示旋转指示器，按钮宽度保持稳定避免布局抖动
//
// 用法示例：
//   Button { text: "确认"; onClicked: doSomething() }
//   Button { text: "取消"; variant: "ghost" }
//   Button { text: "提交"; loading: true }

Item {
    id: root

    property string text:    ""
    property string variant: "filled"   // "filled" | "outline" | "ghost"
    property bool   enabled: true
    property bool   loading: false

    signal clicked()

    implicitWidth:  Math.max(80, label.implicitWidth + 32)
    implicitHeight: ComponentTheme.buttonSize

    // ── 背景 ─────────────────────────────────────────────────
    Rectangle {
        id: bg
        anchors.fill: parent
        radius: ComponentTheme.buttonRadius

        color: {
            if (!root.enabled || root.loading) return _bgBase();
            if (mouse.pressed)         return _bgPressed();
            if (mouse.containsMouse)   return _bgHover();
            return _bgBase();
        }

        border.color: root.variant === "outline"
                      ? (root.enabled ? ComponentTheme.accent : ComponentTheme.accentDisabled)
                      : "transparent"
        border.width: root.variant === "outline" ? 1.5 : 0

        opacity: (!root.enabled || root.loading) ? 0.5 : 1.0

        Behavior on color   { ColorAnimation  { duration: ComponentTheme.durationFast } }
        Behavior on opacity { NumberAnimation { duration: ComponentTheme.durationFast } }
    }

    // ── 文字 ─────────────────────────────────────────────────
    Text {
        id: label
        anchors.centerIn: parent
        visible: !root.loading

        text:           root.text
        font.family:    ComponentTheme.fontFamily
        font.pixelSize: ComponentTheme.fontSize
        font.weight:    ComponentTheme.fontWeightMedium
        color:          root.variant === "filled"
                        ? ComponentTheme.textOnAccent
                        : (root.enabled ? ComponentTheme.accent : ComponentTheme.accentDisabled)
    }

    // ── 加载旋转器（简单 arc 动画）───────────────────────────
    Item {
        id: spinner
        anchors.centerIn: parent
        width: 16; height: 16
        visible: root.loading

        Rectangle {
            anchors.centerIn: parent
            width: 14; height: 14
            radius: 7
            color: "transparent"
            border.width: 2
            border.color: root.variant === "filled"
                          ? ComponentTheme.textOnAccent
                          : ComponentTheme.accent
            opacity: 0.3
        }

        Rectangle {
            id: arc
            anchors.centerIn: parent
            width: 14; height: 14
            radius: 7
            color: "transparent"
            border.width: 2
            border.color: root.variant === "filled"
                          ? ComponentTheme.textOnAccent
                          : ComponentTheme.accent

            // 用旋转遮罩模拟 arc：覆盖 3/4 圆
            layer.enabled: true
            layer.effect: null   // 仅触发 layer 隔离，避免与父级混合

            RotationAnimator on rotation {
                from: 0; to: 360
                duration: 900
                loops: Animation.Infinite
                running: root.loading
            }
        }
    }

    // ── 鼠标 ─────────────────────────────────────────────────
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        enabled:      root.enabled && !root.loading
        cursorShape:  (root.enabled && !root.loading) ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked:    root.clicked()
    }

    // ── 内部辅助：按变体返回各状态背景色 ────────────────────
    function _bgBase() {
        return root.variant === "filled" ? ComponentTheme.accent : "transparent";
    }
    function _bgHover() {
        return root.variant === "filled" ? ComponentTheme.accentHover : ComponentTheme.buttonHover;
    }
    function _bgPressed() {
        return root.variant === "filled" ? ComponentTheme.accentPressed : ComponentTheme.buttonPressed;
    }
}

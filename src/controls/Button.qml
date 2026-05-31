import QtQuick
import QtQuick.Controls.Basic as QQC2
import QuickUI.Components 1.0

// 通用按钮，基于 Qt Quick Controls Button 提供键盘、焦点和状态语义。
//
// 外观变体：
//   variant: "filled"  — 强调色实底（默认）
//   variant: "outline" — 透明底 + 强调色描边
//   variant: "ghost"   — 纯透明，仅显示文字，适合次要操作
//
// 状态：
//   enabled: false — 禁用，阻断交互并降低透明度
//   loading: true  — 显示旋转指示器，默认禁用交互并保持按钮宽度稳定

QQC2.Button {
    id: root

    property string variant: "filled"   // "filled" | "outline" | "ghost"
    property bool loading: false

    enabled: !loading
    implicitWidth: Math.max(80, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: ComponentTheme.buttonSize
    leftPadding: 16
    rightPadding: 16
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true

    background: Rectangle {
        radius: ComponentTheme.buttonRadius
        color: {
            if (!root.enabled) return root._bgBase()
            if (root.down) return root._bgPressed()
            if (root.hovered) return root._bgHover()
            return root._bgBase()
        }
        border.color: root.variant === "outline"
            ? (root.enabled ? ComponentTheme.accent : ComponentTheme.accentDisabled)
            : "transparent"
        border.width: root.variant === "outline" ? 1.5 : 0
        opacity: root.enabled ? 1.0 : 0.5

        Behavior on color { ColorAnimation { duration: ComponentTheme.durationFast } }
        Behavior on opacity { NumberAnimation { duration: ComponentTheme.durationFast } }
    }

    contentItem: Item {
        implicitWidth: Math.max(label.implicitWidth, spinner.implicitWidth)
        implicitHeight: Math.max(label.implicitHeight, spinner.implicitHeight)

        Text {
            id: label
            anchors.centerIn: parent
            visible: !root.loading
            text: root.text
            font.family: ComponentTheme.fontFamily
            font.pixelSize: ComponentTheme.fontSize
            font.weight: ComponentTheme.fontWeightMedium
            color: root.variant === "filled"
                ? ComponentTheme.textOnAccent
                : (root.enabled ? ComponentTheme.accent : ComponentTheme.accentDisabled)
        }

        Item {
            id: spinner
            anchors.centerIn: parent
            implicitWidth: 16
            implicitHeight: 16
            width: 16
            height: 16
            visible: root.loading

            Rectangle {
                anchors.centerIn: parent
                width: 14
                height: 14
                radius: 7
                color: "transparent"
                border.width: 2
                border.color: root.variant === "filled"
                    ? ComponentTheme.textOnAccent
                    : ComponentTheme.accent
                opacity: 0.3
            }

            Rectangle {
                anchors.centerIn: parent
                width: 14
                height: 14
                radius: 7
                color: "transparent"
                border.width: 2
                border.color: root.variant === "filled"
                    ? ComponentTheme.textOnAccent
                    : ComponentTheme.accent

                RotationAnimator on rotation {
                    from: 0
                    to: 360
                    duration: 900
                    loops: Animation.Infinite
                    running: root.loading
                }
            }
        }
    }

    function _bgBase() {
        return root.variant === "filled" ? ComponentTheme.accent : "transparent"
    }

    function _bgHover() {
        return root.variant === "filled" ? ComponentTheme.accentHover : ComponentTheme.buttonHover
    }

    function _bgPressed() {
        return root.variant === "filled" ? ComponentTheme.accentPressed : ComponentTheme.buttonPressed
    }
}

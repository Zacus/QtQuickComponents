import QtQuick
import QtQuick.Controls.Basic as QQC2
import QuickUI.Components 1.0

// 通用图标按钮：基于 Qt Quick Controls Button，保留纯文本/Emoji 图标用法。
QQC2.Button {
    id: root

    property string iconText: "?"
    property string tooltip: ""
    property int fontSize: ComponentTheme.fontSize

    implicitWidth: ComponentTheme.buttonSize
    implicitHeight: ComponentTheme.buttonSize
    hoverEnabled: true

    background: Rectangle {
        radius: ComponentTheme.buttonRadius
        color: root.down
            ? ComponentTheme.buttonPressed
            : root.hovered ? ComponentTheme.buttonHover : "transparent"
        opacity: root.enabled ? 1.0 : 0.4

        Behavior on color { ColorAnimation { duration: ComponentTheme.durationFast } }
        Behavior on opacity { NumberAnimation { duration: ComponentTheme.durationFast } }
    }

    contentItem: Text {
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: root.iconText
        font.pixelSize: root.fontSize
        color: root.down
            ? ComponentTheme.iconColorPressed
            : ComponentTheme.iconColor
    }

    QQC2.ToolTip.visible: root.hovered && root.tooltip.length > 0 && root.enabled
    QQC2.ToolTip.text: root.tooltip
    QQC2.ToolTip.delay: 600
}

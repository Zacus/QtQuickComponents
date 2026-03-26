import QtQuick
import QtQuick.Controls.Basic
import QuickUI.Components 1.0

// 通用图标按钮：纯文本/Emoji 图标 + tooltip + hover 效果
Item {
    id: root

    property string iconText: "?"
    property string tooltip:  ""
    property int    fontSize: ComponentTheme.fontSize
    property bool   enabled:  true

    signal clicked()

    width:  ComponentTheme.buttonSize
    height: ComponentTheme.buttonSize

    Rectangle {
        anchors.fill: parent
        radius: ComponentTheme.buttonRadius
        color:  mouseArea.pressed
                ? ComponentTheme.buttonPressed
                : mouseArea.containsMouse ? ComponentTheme.buttonHover : "transparent"
        opacity: root.enabled ? 1.0 : 0.4

        Behavior on color   { ColorAnimation { duration: ComponentTheme.durationFast } }
        Behavior on opacity { NumberAnimation { duration: ComponentTheme.durationFast } }

        Text {
            anchors.centerIn: parent
            text:        root.iconText
            font.pixelSize: root.fontSize
            color: mouseArea.pressed
                   ? ComponentTheme.iconColorPressed
                   : ComponentTheme.iconColor
        }
    }

    ToolTip.visible: mouseArea.containsMouse && root.tooltip.length > 0 && root.enabled
    ToolTip.text:    root.tooltip
    ToolTip.delay:   600

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        enabled:      root.enabled          // enabled=false 时不接收任何鼠标事件
        cursorShape:  root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked:    root.clicked()
    }
}

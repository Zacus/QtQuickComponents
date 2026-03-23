import QtQuick
import QtQuick.Controls.Basic
import QuickUI.Components 1.0

// 通用图标按钮：纯文本/Emoji 图标 + tooltip + hover 效果
Item {
    id: root

    property string iconText: "?"
    property string tooltip:  ""
    property int    fontSize: ComponentTheme.fontSize

    signal clicked()

    width:  ComponentTheme.buttonSize
    height: ComponentTheme.buttonSize

    Rectangle {
        anchors.fill: parent
        radius: ComponentTheme.buttonRadius
        color:  mouseArea.pressed
                ? ComponentTheme.buttonPressed
                : mouseArea.containsMouse ? ComponentTheme.buttonHover : "transparent"

        Behavior on color { ColorAnimation { duration: ComponentTheme.durationFast } }

        Text {
            anchors.centerIn: parent
            text:        root.iconText
            font.pixelSize: root.fontSize
            color: mouseArea.pressed
                   ? ComponentTheme.iconColorPressed
                   : ComponentTheme.iconColor
        }
    }

    ToolTip.visible: mouseArea.containsMouse && root.tooltip.length > 0
    ToolTip.text:    root.tooltip
    ToolTip.delay:   600

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape:  Qt.PointingHandCursor
        onClicked:    root.clicked()
    }
}

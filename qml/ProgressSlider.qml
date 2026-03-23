import QtQuick
import QtQuick.Controls.Basic
import QuickUI.Components 1.0

// 进度/Seek 滑块：自定义外观，支持点击任意位置跳转
Slider {
    id: root

    from:    0.0
    to:      1.0
    value:   0.0

    implicitHeight: 20

    background: Item {
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width:  parent.width
            height: ComponentTheme.trackHeight
            radius: ComponentTheme.trackHeight / 2
            color:  ComponentTheme.trackBg

            // 已播放部分
            Rectangle {
                width:  root.visualPosition * parent.width
                height: parent.height
                radius: parent.radius
                color:  root.enabled ? ComponentTheme.accent : ComponentTheme.accentDisabled

                Behavior on width { NumberAnimation { duration: 100 } }
            }

            // 鼠标悬停时显示缓冲区
            Rectangle {
                visible: trackMouse.containsMouse
                x:      0
                width:  0.30 * parent.width
                height: parent.height
                radius: parent.radius
                color:  ComponentTheme.trackBuffer
                z:      -1
            }
        }

        MouseArea {
            id: trackMouse
            anchors { fill: parent; topMargin: -8; bottomMargin: -8 }
            hoverEnabled: true
            propagateComposedEvents: true
        }
    }

    handle: Rectangle {
        x: root.leftPadding + root.visualPosition * (root.availableWidth - width)
        y: root.topPadding  + root.availableHeight / 2 - height / 2
        width:  ComponentTheme.handleSize
        height: ComponentTheme.handleSize
        radius: ComponentTheme.handleSize / 2
        color:  root.pressed ? ComponentTheme.accentPressed : ComponentTheme.accent
        border.color: ComponentTheme.handleBorder
        visible: root.enabled

        Behavior on color { ColorAnimation { duration: ComponentTheme.durationFast } }

        scale: root.hovered ? 1.25 : 1.0
        Behavior on scale { NumberAnimation { duration: ComponentTheme.durationNormal } }
    }
}

import QtQuick
import QuickUI.Components 1.0

Item {
    id: root

    property int signalState: WndViewModel.NoSignal
    property string noSignalText: qsTr("无信号")
    property string connectingText: qsTr("连接中")
    property string noSignalIconText: "!"
    property string connectingIconText: ""
    property color textColor: ComponentTheme.textSecondary
    property color iconColor: ComponentTheme.textSecondary

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.45)
    }

    Column {
        anchors.centerIn: parent
        width: Math.max(0, Math.min(parent.width - 16, 160))
        spacing: root.height < 100 ? 4 : 8

        Text {
            id: statusIcon
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.signalState === 2
                ? root.connectingIconText
                : root.noSignalIconText
            color: root.iconColor
            font.pixelSize: root.height < 100 ? 18 : 26
            visible: text.length > 0

            RotationAnimation on rotation {
                running: root.visible
                    && root.signalState === 2
                    && root.connectingIconText.length > 0
                from: 0
                to: 360
                duration: 900
                loops: Animation.Infinite
            }
        }

        Text {
            objectName: "noSignalText"
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: root.signalState === 2
                ? root.connectingText
                : root.noSignalText
            color: root.textColor
            font.family: ComponentTheme.fontFamily
            font.pixelSize: root.height < 100 ? ComponentTheme.fontSizeCaption : ComponentTheme.fontSizeLabel
        }
    }
}

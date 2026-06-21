import QtQuick
import QuickUI.Components 1.0

Item {
    id: root

    property string channelName: ""
    property bool isRecording: false
    property bool expanded: false
    property color backgroundColor: Qt.rgba(0, 0, 0, 0.62)
    property color textColor: "#ffffff"

    signal screenshotClicked()
    signal recordClicked()
    signal closeClicked()

    height: 32
    opacity: expanded ? 1.0 : 0.0
    enabled: opacity > 0.0

    Behavior on opacity {
        NumberAnimation { duration: 120 }
    }

    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    Text {
        id: titleText
        objectName: "titleText"
        anchors {
            left: parent.left
            leftMargin: 8
            right: buttonRow.left
            rightMargin: 8
            verticalCenter: parent.verticalCenter
        }
        text: root.channelName
        color: root.textColor
        elide: Text.ElideRight
        font.pixelSize: ComponentTheme.fontSizeCaption
        verticalAlignment: Text.AlignVCenter
    }

    Row {
        id: buttonRow
        anchors {
            right: parent.right
            rightMargin: 4
            verticalCenter: parent.verticalCenter
        }
        spacing: 2

        IconButton {
            objectName: "screenshotButton"
            width: 28
            height: 28
            iconText: "S"
            tooltip: qsTr("截屏")
            fontSize: 13
            onClicked: root.screenshotClicked()
        }

        IconButton {
            objectName: "recordButton"
            width: 28
            height: 28
            iconText: "R"
            tooltip: qsTr("录制")
            fontSize: 13
            onClicked: root.recordClicked()

            Rectangle {
                objectName: "recordingIndicator"
                anchors {
                    right: parent.right
                    top: parent.top
                    margins: 5
                }
                width: 6
                height: 6
                radius: 3
                color: "#eb5757"
                visible: root.isRecording
            }
        }

        IconButton {
            objectName: "closeButton"
            width: 28
            height: 28
            iconText: "X"
            tooltip: qsTr("关闭")
            fontSize: 13
            onClicked: root.closeClicked()
        }
    }
}

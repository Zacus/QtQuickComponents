import QtQuick
import QuickUI.Components 1.0
import QuickUI.Components.impl 1.0

Item {
    id: root

    property WndViewModel vm: null
    property alias videoSurface: videoSurface

    implicitWidth: 320
    implicitHeight: 180
    clip: true
    opacity: dragHandler.active ? 0.62 : 1.0

    Rectangle {
        anchors.fill: parent
        color: "#05070a"
        z: -1
    }

    VideoSurface {
        id: videoSurface
        objectName: "videoSurface"
        anchors.fill: parent
        z: 0
        channelId: root.vm ? root.vm.channelId : -1
        videoSink: root.vm ? root.vm.videoSink : null
        visible: root.vm && root.vm.signalState === WndViewModel.Normal
    }

    NoSignalLayer {
        id: noSignalLayer
        objectName: "noSignalLayer"
        anchors.fill: parent
        z: 1
        signalState: root.vm ? root.vm.signalState : WndViewModel.NoSignal
        visible: root.vm
            ? root.vm.signalState !== WndViewModel.Normal
            : true
    }

    OSDLayer {
        id: osdLayer
        objectName: "osdLayer"
        anchors.fill: parent
        z: 2
        osdModel: root.vm ? root.vm.osdModel : []
    }

    BorderLayer {
        id: borderLayer
        objectName: "borderLayer"
        anchors.fill: parent
        z: 3
        isActive: root.vm ? root.vm.isActive : false
        alarmLevel: root.vm ? root.vm.alarmLevel : WndViewModel.None
    }

    TitleBar {
        id: titleBar
        objectName: "titleBar"
        anchors { left: parent.left; right: parent.right; top: parent.top }
        z: 4
        expanded: hoverHandler.hovered
        channelName: root.vm ? root.vm.channelName : ""
        isRecording: root.vm ? root.vm.isRecording : false
        onScreenshotClicked: {
            if (root.vm)
                root.vm.screenshotRequested(root.vm.wndId, root.vm.channelId)
        }
        onRecordClicked: {
            if (root.vm)
                root.vm.recordToggled(root.vm.wndId, root.vm.channelId)
        }
        onCloseClicked: {
            if (root.vm)
                root.vm.channelClosed(root.vm.wndId, root.vm.channelId)
        }
    }

    Rectangle {
        id: dragLayer
        objectName: "dragLayer"
        anchors.fill: parent
        z: 5
        color: "transparent"
        border.width: 0
    }

    MouseArea {
        id: clickArea
        anchors.fill: parent
        z: 3.5
        acceptedButtons: Qt.LeftButton
        hoverEnabled: false
        propagateComposedEvents: true
        property bool suppressNextClick: false
        onClicked: function(mouse) {
            if (suppressNextClick) {
                suppressNextClick = false
                mouse.accepted = false
                return
            }
            clickConfirmTimer.restart()
            mouse.accepted = false
        }
        onDoubleClicked: function(mouse) {
            clickConfirmTimer.stop()
            suppressNextClick = true
            if (root.vm)
                root.vm.doubleClicked(root.vm.wndId, root.vm.channelId)
            mouse.accepted = false
        }
    }

    Timer {
        id: clickConfirmTimer
        interval: 220
        repeat: false
        onTriggered: {
            if (root.vm)
                root.vm.clicked(root.vm.wndId)
        }
    }

    DragHandler {
        id: dragHandler
        target: null
        acceptedButtons: Qt.LeftButton
        onActiveChanged: {
            if (!active)
                return

            clickConfirmTimer.stop()
            clickArea.suppressNextClick = true
            if (root.vm && root.vm.channelId >= 0)
                root.vm.dragStarted(root.vm.wndId, root.vm.channelId)
        }
    }

    HoverHandler {
        id: hoverHandler
    }
}

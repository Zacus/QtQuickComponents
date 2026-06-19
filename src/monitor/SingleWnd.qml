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
        anchors.fill: parent
        z: 3.5
        acceptedButtons: Qt.LeftButton
        hoverEnabled: false
        propagateComposedEvents: true
        onClicked: function(mouse) {
            if (root.vm)
                root.vm.clicked(root.vm.wndId)
            mouse.accepted = false
        }
        onDoubleClicked: function(mouse) {
            if (root.vm)
                root.vm.doubleClicked(root.vm.wndId, root.vm.channelId)
            mouse.accepted = false
        }
    }

    HoverHandler {
        id: hoverHandler
    }
}

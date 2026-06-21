pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as QQC2
import QtQuick.Layouts
import QuickUI.Components 1.0
import QuickUI.Components.impl 1.0
import SingleWndDemo 1.0

QQC2.ApplicationWindow {
    id: window

    width: 1180
    height: 760
    minimumWidth: 960
    minimumHeight: 620
    visible: true
    title: "SingleWnd Demo"
    color: "#10131a"

    property int selectedIndex: 0
    property int maximizedWndId: 0
    readonly property var viewModels: [vm1, vm2, vm3, vm4]
    readonly property var selectedVm: viewModels[selectedIndex]
    readonly property var channelList: [vm1.channelId, vm2.channelId, vm3.channelId, vm4.channelId]
    property int pushedFrames: 0
    property string lastFrameText: "0"

    function stateName(state) {
        if (state === WndViewModel.Normal)
            return "Normal"
        if (state === WndViewModel.Connecting)
            return "Connecting"
        return "No signal"
    }

    function alarmName(level) {
        if (level === WndViewModel.Critical)
            return "Critical"
        if (level === WndViewModel.Warning)
            return "Warning"
        return "None"
    }

    function osdFor(vm) {
        return [
            { text: "WND-" + vm.wndId, position: 0, color: "white" },
            { text: "CH-" + vm.channelId, position: 1, color: "#7dd3fc" },
            { text: vm.isRecording ? "REC" : "", position: 2, color: "#ff4d4d" },
            { text: stateName(vm.signalState), position: 3, color: "#d1d5db" }
        ]
    }

    function refreshOsd(vm) {
        vm.osdModel = osdFor(vm)
    }

    function vmForWndId(wndId) {
        for (var i = 0; i < viewModels.length; ++i) {
            if (viewModels[i].wndId === wndId)
                return viewModels[i]
        }
        return null
    }

    function indexForWndId(wndId) {
        for (var i = 0; i < viewModels.length; ++i) {
            if (viewModels[i].wndId === wndId)
                return i
        }
        return -1
    }

    function vmForChannelId(channelId) {
        for (var i = 0; i < viewModels.length; ++i) {
            if (viewModels[i].channelId === channelId)
                return viewModels[i]
        }
        return null
    }

    function addLog(text) {
        logModel.insert(0, {
            message: Qt.formatTime(new Date(), "hh:mm:ss.zzz") + "  " + text
        })
        while (logModel.count > 80)
            logModel.remove(logModel.count - 1)
    }

    function selectWindow(index) {
        selectedIndex = index
        for (var i = 0; i < viewModels.length; ++i)
            viewModels[i].isActive = i === index
    }

    function selectWindowByWndId(wndId) {
        var index = indexForWndId(wndId)
        if (index >= 0)
            selectWindow(index)
    }

    function toggleMaximized(wndId) {
        selectWindowByWndId(wndId)
        maximizedWndId = maximizedWndId === wndId ? 0 : wndId
        addLog(maximizedWndId === 0 ? "layout restored" : "maximized wnd=" + wndId)
    }

    function handleDrop(wndId, fromChannelId) {
        var target = vmForWndId(wndId)
        var source = vmForChannelId(fromChannelId)
        if (!target || !source || target === source)
            return

        var targetChannelId = target.channelId
        var targetChannelName = target.channelName
        target.channelId = source.channelId
        target.channelName = source.channelName
        source.channelId = targetChannelId
        source.channelName = targetChannelName
        refreshOsd(target)
        refreshOsd(source)
        addLog("dropReceived wnd=" + wndId + " fromChannel=" + fromChannelId)
    }

    function connectVmSignals(vm) {
        vm.clicked.connect(function(wndId) {
            selectWindowByWndId(wndId)
            addLog("clicked wnd=" + wndId)
        })
        vm.doubleClicked.connect(function(wndId, channelId) {
            addLog("doubleClicked wnd=" + wndId + " channel=" + channelId)
            toggleMaximized(wndId)
        })
        vm.screenshotRequested.connect(function(wndId, channelId) {
            addLog("screenshotRequested wnd=" + wndId + " channel=" + channelId)
        })
        vm.recordToggled.connect(function(wndId, channelId) {
            vm.isRecording = !vm.isRecording
            refreshOsd(vm)
            addLog("recordToggled wnd=" + wndId + " channel=" + channelId)
        })
        vm.channelClosed.connect(function(wndId, channelId) {
            vm.signalState = WndViewModel.NoSignal
            refreshOsd(vm)
            addLog("channelClosed wnd=" + wndId + " channel=" + channelId)
        })
        vm.dragStarted.connect(function(wndId, channelId) {
            selectWindowByWndId(wndId)
            addLog("dragStarted wnd=" + wndId + " channel=" + channelId)
        })
        vm.dropReceived.connect(function(wndId, fromChannelId) {
            handleDrop(wndId, fromChannelId)
        })
    }

    Component.onCompleted: {
        ComponentTheme.style = ComponentTheme.Dark
        for (var i = 0; i < viewModels.length; ++i) {
            refreshOsd(viewModels[i])
            connectVmSignals(viewModels[i])
        }
        selectWindow(0)
        addLog("demoReady")
    }

    GlobalVideoRenderer {
        id: renderer
    }

    FramePump {
        id: framePump
        videoSink: renderer
        channels: window.channelList
        running: streamToggle.checked
        interval: frameRateSlider.value
        frameWidth: 320
        frameHeight: 180
        onFramePushed: function(channelId, serial) {
            ++window.pushedFrames
            window.lastFrameText = "ch " + channelId + " #" + serial
        }
    }

    WndViewModel {
        id: vm1
        wndId: 1
        channelId: 101
        channelName: "Entrance"
        signalState: WndViewModel.Normal
        videoSink: renderer
    }

    WndViewModel {
        id: vm2
        wndId: 2
        channelId: 102
        channelName: "Lobby"
        signalState: WndViewModel.Normal
        videoSink: renderer
    }

    WndViewModel {
        id: vm3
        wndId: 3
        channelId: 103
        channelName: "Warehouse"
        signalState: WndViewModel.NoSignal
        videoSink: renderer
    }

    WndViewModel {
        id: vm4
        wndId: 4
        channelId: 104
        channelName: "Loading Dock"
        signalState: WndViewModel.Connecting
        videoSink: renderer
    }

    ListModel {
        id: logModel
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 18

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Label {
                    Layout.fillWidth: true
                    role: "heading"
                    text: "SingleWnd"
                }

                Label {
                    role: "caption"
                    text: window.lastFrameText
                }

                QQC2.Button {
                    id: streamToggle
                    checkable: true
                    checked: true
                    text: checked ? "Streaming" : "Stopped"
                }

                Button {
                    visible: window.maximizedWndId !== 0
                    text: "Exit Max"
                    variant: "outline"
                    onClicked: {
                        window.maximizedWndId = 0
                        window.addLog("layout restored")
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 2
                rowSpacing: 10
                columnSpacing: 10

                Repeater {
                    model: window.viewModels.length

                    delegate: Item {
                        id: tile
                        required property int index
                        readonly property var tileVm: window.viewModels[index]
                        readonly property bool maximized: window.maximizedWndId === tile.tileVm.wndId

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: 260
                        Layout.minimumHeight: 160
                        Layout.columnSpan: tile.maximized ? 2 : 1
                        Layout.rowSpan: tile.maximized ? 2 : 1
                        visible: window.maximizedWndId === 0 || tile.maximized

                        SingleWnd {
                            id: wnd
                            anchors.fill: parent
                            vm: tile.tileVm
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.width: window.selectedIndex === tile.index ? 2 : 0
                            border.color: "#38bdf8"
                            enabled: false
                            z: 8
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 360
            Layout.fillHeight: true
            color: "#171b25"
            border.color: "#2d3342"
            radius: 6

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

                Label {
                    role: "label"
                    text: "Window " + window.selectedVm.wndId + " / Channel " + window.selectedVm.channelId
                }

                QQC2.ComboBox {
                    Layout.fillWidth: true
                    model: ["Normal", "No signal", "Connecting"]
                    currentIndex: window.selectedVm.signalState
                    onActivated: function(index) {
                        window.selectedVm.signalState = index
                        window.refreshOsd(window.selectedVm)
                        window.addLog("state " + window.stateName(index))
                    }
                }

                QQC2.ComboBox {
                    Layout.fillWidth: true
                    model: ["None", "Warning", "Critical"]
                    currentIndex: window.selectedVm.alarmLevel
                    onActivated: function(index) {
                        window.selectedVm.alarmLevel = index
                        window.addLog("alarm " + window.alarmName(index))
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    QQC2.CheckBox {
                        text: "Active"
                        checked: window.selectedVm.isActive
                        onToggled: {
                            window.selectedVm.isActive = checked
                            if (checked)
                                window.selectedIndex = window.viewModels.indexOf(window.selectedVm)
                        }
                    }

                    QQC2.CheckBox {
                        text: "Recording"
                        checked: window.selectedVm.isRecording
                        onToggled: {
                            window.selectedVm.isRecording = checked
                            window.refreshOsd(window.selectedVm)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Button {
                        Layout.fillWidth: true
                        text: "OSD"
                        variant: "outline"
                        onClicked: {
                            window.refreshOsd(window.selectedVm)
                            window.addLog("osd refreshed")
                        }
                    }

                    Button {
                        Layout.fillWidth: true
                        text: "Clear"
                        variant: "ghost"
                        onClicked: {
                            window.selectedVm.osdModel = []
                            window.addLog("osd cleared")
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#2d3342"
                }

                Label {
                    role: "caption"
                    text: "Frames " + window.pushedFrames
                }

                QQC2.Slider {
                    id: frameRateSlider
                    Layout.fillWidth: true
                    from: 40
                    to: 300
                    stepSize: 20
                    value: 120
                }

                Label {
                    role: "caption"
                    text: "Interval " + Math.round(frameRateSlider.value) + " ms"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#2d3342"
                }

                Label {
                    role: "label"
                    text: "Events"
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: logModel

                    delegate: Text {
                        required property string message
                        width: ListView.view.width
                        text: message
                        color: "#d1d5db"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}

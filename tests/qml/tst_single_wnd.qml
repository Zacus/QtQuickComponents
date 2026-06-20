import QtQuick
import QtTest
import QuickUI.Components 1.0
import QuickUI.Components.impl 1.0

TestCase {
    name: "SingleWnd"
    when: windowShown
    width: 640
    height: 360
    visible: true

    Component {
        id: singleWndComponent

        SingleWnd {
            width: 320
            height: 180
            visible: true

            vm: WndViewModel {
                wndId: 3
                channelId: 12
                channelName: "Lobby entrance camera with a long name"
                signalState: WndViewModel.NoSignal
                osdModel: [
                    { text: "CAM-12", position: 0, color: "white" },
                    { text: "REC", position: 1, color: "red" }
                ]
            }
        }
    }

    Component {
        id: noSignalLayerComponent

        NoSignalLayer {
            width: 120
            height: 90
            noSignalText: "offline"
            connectingText: "connecting"
            noSignalIconText: "N"
            connectingIconText: "C"
        }
    }

    Component {
        id: borderLayerComponent

        BorderLayer {
            width: 120
            height: 90
        }
    }

    function child(root, objectName) {
        var result = findChild(root, objectName)
        verify(result !== null, "Missing child " + objectName)
        return result
    }

    function test_layersFollowViewModelState() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        compare(child(wnd, "videoSurface").visible, false)
        compare(child(wnd, "noSignalLayer").signalState, WndViewModel.NoSignal)
        compare(child(wnd, "noSignalLayer").visible, true)

        wnd.vm.signalState = WndViewModel.Normal
        compare(child(wnd, "videoSurface").visible, true)
        compare(child(wnd, "noSignalLayer").visible, false)
        compare(child(wnd, "videoSurface").channelId, 12)
        compare(child(wnd, "videoSurface").hasFrame, false)
        compare(child(wnd, "videoSurface").currentSerial, 0)
        compare(child(wnd, "videoSurface").activeFrameFormat, 0)
    }

    function test_osdModelRendersText() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        compare(child(wnd, "osdTopLeftText").text, "CAM-12")
        compare(child(wnd, "osdTopRightText").text, "REC")

        wnd.vm.osdModel = []
        compare(child(wnd, "osdTopLeftText").text, "")
        compare(child(wnd, "osdTopRightText").text, "")
    }

    function test_titleButtonsForwardViewModelSignals() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)
        child(wnd, "titleBar").expanded = true
        wait(150)

        var screenshotSpy = signalSpy.createObject(this, {
            target: wnd.vm,
            signalName: "screenshotRequested"
        })
        var recordSpy = signalSpy.createObject(this, {
            target: wnd.vm,
            signalName: "recordToggled"
        })
        var closeSpy = signalSpy.createObject(this, {
            target: wnd.vm,
            signalName: "channelClosed"
        })

        mouseClick(child(wnd, "screenshotButton"))
        mouseClick(child(wnd, "recordButton"))
        mouseClick(child(wnd, "closeButton"))

        compare(screenshotSpy.count, 1)
        compare(screenshotSpy.signalArguments[0][0], 3)
        compare(screenshotSpy.signalArguments[0][1], 12)
        compare(recordSpy.count, 1)
        compare(closeSpy.count, 1)
    }

    function test_titleBarRecordingIndicatorFollowsViewModel() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        var indicator = child(wnd, "recordingIndicator")
        compare(indicator.visible, false)

        wnd.vm.isRecording = true
        compare(indicator.visible, true)

        wnd.vm.isRecording = false
        compare(indicator.visible, false)
    }

    function test_singleClickForwardsClickedSignal() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        var clickSpy = signalSpy.createObject(this, {
            target: wnd.vm,
            signalName: "clicked"
        })

        mouseClick(wnd, 20, 80)
        wait(260)

        compare(clickSpy.count, 1)
        compare(clickSpy.signalArguments[0][0], 3)
    }

    function test_doubleClickForwardsOnlyDoubleClickedSignal() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        var clickSpy = signalSpy.createObject(this, {
            target: wnd.vm,
            signalName: "clicked"
        })
        var doubleClickSpy = signalSpy.createObject(this, {
            target: wnd.vm,
            signalName: "doubleClicked"
        })

        mouseDoubleClickSequence(wnd, 20, 80)
        wait(260)

        compare(clickSpy.count, 0)
        compare(doubleClickSpy.count, 1)
        compare(doubleClickSpy.signalArguments[0][0], 3)
        compare(doubleClickSpy.signalArguments[0][1], 12)
    }

    function test_dragStartReportsSignalAndRestoresVisualState() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        var dragSpy = signalSpy.createObject(this, {
            target: wnd.vm,
            signalName: "dragStarted"
        })

        compare(wnd.opacity, 1.0)

        mousePress(wnd, 20, 80)
        mouseMove(wnd, 80, 100)
        wait(80)

        compare(dragSpy.count, 1)
        compare(dragSpy.signalArguments[0][0], 3)
        compare(dragSpy.signalArguments[0][1], 12)
        verify(wnd.opacity < 1.0)

        mouseRelease(wnd, 80, 100)
        wait(80)
        compare(wnd.opacity, 1.0)
    }

    function test_noSignalLayerShowsOnlyNonNormalStates() {
        var layer = createTemporaryObject(noSignalLayerComponent, this)
        verify(layer !== null)

        var label = child(layer, "noSignalText")
        layer.signalState = WndViewModel.NoSignal
        compare(label.text, "offline")
        compare(label.visible, true)

        layer.signalState = WndViewModel.Connecting
        compare(label.text, "connecting")
        compare(label.visible, true)

        layer.signalState = WndViewModel.Normal
        compare(label.text, "")
        compare(label.visible, false)
    }

    function test_borderLayerAppliesPriorityAndHidesWhenInactive() {
        var layer = createTemporaryObject(borderLayerComponent, this)
        verify(layer !== null)

        compare(layer.border.width, 0)
        compare(layer.visible, false)

        layer.isActive = true
        compare(layer.visible, true)
        compare(layer.border.width, 2)
        verify(Qt.colorEqual(layer.border.color, layer.activeColor))

        layer.alarmLevel = WndViewModel.Warning
        verify(Qt.colorEqual(layer.border.color, layer.warningColor))

        layer.alarmLevel = WndViewModel.Critical
        verify(Qt.colorEqual(layer.border.color, layer.criticalColor))

        layer.alarmLevel = WndViewModel.None
        layer.isActive = false
        compare(layer.border.width, 0)
        compare(layer.visible, false)
    }

    Component {
        id: signalSpy
        SignalSpy {}
    }
}

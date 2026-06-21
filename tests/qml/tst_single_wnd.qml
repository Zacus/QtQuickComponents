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
        id: dualWndComponent

        Item {
            width: 640
            height: 180
            visible: true

            SingleWnd {
                id: sourceWnd
                objectName: "sourceWnd"
                width: 320
                height: 180
                visible: true

                vm: WndViewModel {
                    wndId: 3
                    channelId: 12
                    channelName: "Source camera"
                    signalState: WndViewModel.Normal
                }
            }

            SingleWnd {
                id: targetWnd
                objectName: "targetWnd"
                x: 320
                width: 320
                height: 180
                visible: true

                vm: WndViewModel {
                    wndId: 4
                    channelId: 18
                    channelName: "Target camera"
                    signalState: WndViewModel.Normal
                }
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
        id: defaultNoSignalLayerComponent

        NoSignalLayer {
            width: 120
            height: 90
        }
    }

    Component {
        id: compactNoSignalLayerComponent

        NoSignalLayer {
            width: 120
            height: 90
            noSignalText: "offline camera disconnected in loading dock"
            connectingText: "connecting to upstream video service"
            noSignalIconText: "CUSTOM_OFFLINE_ICON"
            connectingIconText: "CUSTOM_CONNECTING_ICON"
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

    function textItem(root, text) {
        if (root && root.text === text)
            return root

        for (var i = 0; root && root.children && i < root.children.length; ++i) {
            var result = textItem(root.children[i], text)
            if (result)
                return result
        }
        return null
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

    function test_osdModelRendersFourCornersAndUpdatesColors() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        wnd.vm.osdModel = [
            { text: "TOP-LEFT", position: 0, color: "#ff0000" },
            { text: "TOP-RIGHT", position: 1, color: "#00ff00" },
            { text: "BOTTOM-LEFT", position: 2, color: "#0000ff" },
            { text: "BOTTOM-RIGHT", position: 3, color: "#ffff00" }
        ]

        var topLeft = child(wnd, "osdTopLeftText")
        var topRight = child(wnd, "osdTopRightText")
        var bottomLeft = child(wnd, "osdBottomLeftText")
        var bottomRight = child(wnd, "osdBottomRightText")

        compare(topLeft.text, "TOP-LEFT")
        compare(topRight.text, "TOP-RIGHT")
        compare(bottomLeft.text, "BOTTOM-LEFT")
        compare(bottomRight.text, "BOTTOM-RIGHT")
        verify(Qt.colorEqual(topLeft.color, "#ff0000"))
        verify(Qt.colorEqual(topRight.color, "#00ff00"))
        verify(Qt.colorEqual(bottomLeft.color, "#0000ff"))
        verify(Qt.colorEqual(bottomRight.color, "#ffff00"))

        wnd.vm.osdModel = [
            { text: "UPDATED-LEFT", position: 0, color: "#00ffff" },
            { text: "UPDATED-BOTTOM", position: 3, color: "#ff00ff" }
        ]

        compare(topLeft.text, "UPDATED-LEFT")
        compare(topRight.text, "")
        compare(bottomLeft.text, "")
        compare(bottomRight.text, "UPDATED-BOTTOM")
        verify(Qt.colorEqual(topLeft.color, "#00ffff"))
        verify(Qt.colorEqual(bottomRight.color, "#ff00ff"))
    }

    function test_osdModelEmptyAndLongTextStayInsideBounds() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        var topLeft = child(wnd, "osdTopLeftText")
        var topRight = child(wnd, "osdTopRightText")
        var bottomLeft = child(wnd, "osdBottomLeftText")
        var bottomRight = child(wnd, "osdBottomRightText")

        wnd.vm.osdModel = []
        compare(topLeft.visible, false)
        compare(topRight.visible, false)
        compare(bottomLeft.visible, false)
        compare(bottomRight.visible, false)

        var longText = "VERY-LONG-OSD-VALUE-WITHOUT-SPACES-0123456789"
        wnd.vm.osdModel = [
            { text: longText + "-TL", position: 0, color: "white" },
            { text: longText + "-TR", position: 1, color: "white" },
            { text: longText + "-BL", position: 2, color: "white" },
            { text: longText + "-BR", position: 3, color: "white" }
        ]

        verify(topLeft.paintedWidth <= topLeft.width)
        verify(topRight.paintedWidth <= topRight.width)
        verify(bottomLeft.paintedWidth <= bottomLeft.width)
        verify(bottomRight.paintedWidth <= bottomRight.width)
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

    function test_titleBarHoverAndLongChannelNameStayBounded() {
        var wnd = createTemporaryObject(singleWndComponent, this)
        verify(wnd !== null)

        var titleBar = child(wnd, "titleBar")
        compare(titleBar.expanded, false)

        mouseMove(wnd, 20, 20)
        wait(80)
        compare(titleBar.expanded, true)

        var titleText = child(wnd, "titleText")
        compare(titleText.text, wnd.vm.channelName)
        verify(titleText.width > 0)
        verify(titleText.paintedWidth <= titleText.width)

        mouseMove(wnd, wnd.width + 20, wnd.height + 20)
        wait(80)
        compare(titleBar.expanded, false)
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

    function test_dragTargetHighlightsWhileSourceHovers() {
        var fixture = createTemporaryObject(dualWndComponent, this)
        verify(fixture !== null)

        var source = child(fixture, "sourceWnd")
        var target = child(fixture, "targetWnd")
        var targetDragLayer = child(target, "dragLayer")

        compare(targetDragLayer.border.width, 0)

        mousePress(source, 20, 80)
        mouseMove(source, 360, 80)
        wait(80)

        verify(targetDragLayer.border.width > 0)

        mouseMove(source, 20, 80)
        wait(80)
        compare(targetDragLayer.border.width, 0)

        mouseRelease(source, 20, 80)
    }

    function test_dropOnTargetReportsTargetWndAndSourceChannel() {
        var fixture = createTemporaryObject(dualWndComponent, this)
        verify(fixture !== null)

        var source = child(fixture, "sourceWnd")
        var target = child(fixture, "targetWnd")
        var dropSpy = signalSpy.createObject(this, {
            target: target.vm,
            signalName: "dropReceived"
        })

        mousePress(source, 20, 80)
        mouseMove(source, 360, 80)
        wait(80)
        mouseRelease(source, 360, 80)
        wait(80)

        compare(dropSpy.count, 1)
        compare(dropSpy.signalArguments[0][0], 4)
        compare(dropSpy.signalArguments[0][1], 12)
        compare(child(target, "dragLayer").border.width, 0)
    }

    function test_escapeCancelsDragAndRestoresVisualState() {
        var fixture = createTemporaryObject(dualWndComponent, this)
        verify(fixture !== null)

        var source = child(fixture, "sourceWnd")
        var target = child(fixture, "targetWnd")
        var targetDragLayer = child(target, "dragLayer")
        var dropSpy = signalSpy.createObject(this, {
            target: target.vm,
            signalName: "dropReceived"
        })

        mousePress(source, 20, 80)
        mouseMove(source, 360, 80)
        wait(80)

        verify(source.opacity < 1.0)
        verify(targetDragLayer.border.width > 0)
        source.forceActiveFocus()

        keyClick(Qt.Key_Escape)
        wait(80)

        compare(dropSpy.count, 0)
        compare(source.opacity, 1.0)
        compare(targetDragLayer.border.width, 0)

        mouseRelease(source, 360, 80)
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

    function test_noSignalLayerProvidesDefaultConnectingIcon() {
        var layer = createTemporaryObject(defaultNoSignalLayerComponent, this)
        verify(layer !== null)

        layer.signalState = WndViewModel.Connecting
        verify(layer.connectingIconText.length > 0)
    }

    function test_noSignalLayerKeepsCustomContentInsideCompactBounds() {
        var layer = createTemporaryObject(compactNoSignalLayerComponent, this)
        verify(layer !== null)

        layer.signalState = WndViewModel.NoSignal
        var noSignalIcon = textItem(layer, layer.noSignalIconText)
        verify(noSignalIcon !== null)
        verify(noSignalIcon.paintedWidth <= noSignalIcon.width)
        verify(child(layer, "noSignalText").paintedWidth <= child(layer, "noSignalText").width)

        layer.signalState = WndViewModel.Connecting
        var connectingIcon = textItem(layer, layer.connectingIconText)
        verify(connectingIcon !== null)
        verify(connectingIcon.paintedWidth <= connectingIcon.width)
        verify(child(layer, "noSignalText").paintedWidth <= child(layer, "noSignalText").width)
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

    function test_borderLayerAlarmOverridesActiveAndCriticalPulseRuns() {
        var layer = createTemporaryObject(borderLayerComponent, this)
        verify(layer !== null)

        layer.isActive = true
        verify(Qt.colorEqual(layer.border.color, layer.activeColor))

        layer.alarmLevel = WndViewModel.Warning
        compare(layer.visible, true)
        compare(layer.border.width, 2)
        verify(Qt.colorEqual(layer.border.color, layer.warningColor))

        layer.alarmLevel = WndViewModel.Critical
        compare(layer.visible, true)
        compare(layer.border.width, 2)
        verify(Qt.colorEqual(layer.border.color, layer.criticalColor))
        compare(child(layer, "criticalPulseAnimation").running, true)

        layer.alarmLevel = WndViewModel.None
        layer.isActive = false
        compare(child(layer, "criticalPulseAnimation").running, false)
        compare(layer.border.width, 0)
        compare(layer.visible, false)
    }

    Component {
        id: signalSpy
        SignalSpy {}
    }
}

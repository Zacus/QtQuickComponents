import QtQuick
import QtTest
import QuickUI.Components 1.0
import QuickUI.Components.impl 1.0 as TimelineImpl

TestCase {
    name: "PlayheadOverlay"

    Component {
        id: playheadComponent

        TimelineImpl.PlayheadOverlay {
            width: 200
            height: 60
            currentTime: 500

            viewport: TimelineImpl.TimelineViewport {
                viewWidth: 200
                totalStart: 0
                totalEnd: 1000
                Component.onCompleted: fitAll()
            }
        }
    }

    function test_defaultHeadPositionUsesEnum() {
        var overlay = createTemporaryObject(playheadComponent, this)

        verify(overlay !== null)
        compare(overlay.headPosition, TimelineEnums.HeadTop)
    }
}

import QtQuick
import QtTest
import QuickUI.Components 1.0

TestCase {
    name: "PlayheadOverlay"

    Component {
        id: playheadComponent

        PlayheadOverlay {
            width: 200
            height: 60
            currentTime: 500

            viewport: TimelineViewport {
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

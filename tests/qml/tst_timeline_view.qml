import QtQuick
import QtTest
import QuickUI.Components 1.0
import QuickUI.Components.impl 1.0 as TimelineImpl

TestCase {
    name: "TimelineView"
    when: windowShown
    width: 640
    height: 240
    visible: true

    Component {
        id: timelineViewComponent

        Item {
            width: 480
            height: 120
            visible: true

            TimelineModel {
                id: timelineModel
                Component.onCompleted: {
                    addSegment(0, 900000, TimelineEnums.SegmentNormal)
                    addSegment(1200000, 2300000, TimelineEnums.SegmentMotion)
                    addSegment(2600000, 3500000, TimelineEnums.SegmentNormal)
                }
            }

            TimelineView {
                id: timelineView
                objectName: "timelineView"
                anchors.fill: parent
                visible: true
                model: timelineModel
            }
        }
    }

    Component {
        id: directTimelineViewComponent

        TimelineView {
            id: timelineView
            objectName: "timelineView"
            width: 480
            height: 120
            visible: true
            model: timelineModel

            TimelineModel {
                id: timelineModel
                Component.onCompleted: {
                    addSegment(0, 900000, TimelineEnums.SegmentNormal)
                    addSegment(1200000, 2300000, TimelineEnums.SegmentMotion)
                    addSegment(2600000, 3500000, TimelineEnums.SegmentNormal)
                }
            }
        }
    }

    Component {
        id: themedSurfaceTimelineViewComponent

        Rectangle {
            width: 480
            height: 120
            visible: true
            color: ComponentTheme.surface

            TimelineModel {
                id: timelineModel
                Component.onCompleted: {
                    addSegment(0, 900000, TimelineEnums.SegmentNormal)
                    addSegment(1200000, 2300000, TimelineEnums.SegmentMotion)
                    addSegment(2600000, 3500000, TimelineEnums.SegmentNormal)
                }
            }

            TimelineView {
                id: timelineView
                objectName: "timelineView"
                anchors.fill: parent
                visible: true
                model: timelineModel
            }
        }
    }

    Component {
        id: rulerModelComponent

        Item {
            width: 480
            height: 120
            visible: true

            TimelineImpl.TimelineViewport {
                id: viewport
                viewWidth: 480
                totalStart: 0
                totalEnd: 3600000
                Component.onCompleted: fitAll()
            }

            TimelineImpl.RulerModel {
                id: rulerModel
                objectName: "rulerModel"
                viewport: viewport
            }
        }
    }

    Component {
        id: rulerPaintComponent

        TimelineImpl.TimelineRuler {
            id: ruler
            width: 480
            height: 40
            visible: true
            backgroundColor: "black"
            majorTickColor: "red"
            minorTickColor: "green"
            labelColor: "blue"
            separatorColor: "yellow"
            labelFont: "11px sans-serif"

            viewport: TimelineImpl.TimelineViewport {
                id: viewport
                viewWidth: ruler.width
                totalStart: 0
                totalEnd: 3600000
                Component.onCompleted: fitAll()
            }

            rulerModel: TimelineImpl.RulerModel {
                viewport: viewport
            }
        }
    }

    function test_timelineViewAutoFitsInitialModelData() {
        var root = createTemporaryObject(timelineViewComponent, this)
        verify(root !== null)

        var timelineView = findChild(root, "timelineView")
        verify(timelineView !== null)
        tryVerify(function() { return timelineView.viewport.viewSpan > 0 })
        verify(timelineView.viewport.pixelsPerMs > 0)
    }

    function test_timelineViewDefaultColorsFollowTheme() {
        ComponentTheme.style = ComponentTheme.Dark
        var timelineView = createTemporaryObject(directTimelineViewComponent, this)
        verify(timelineView !== null)

        compare(timelineView.trackColor, ComponentTheme.trackBg)
        compare(timelineView.rulerBg, ComponentTheme.trackBg)
        compare(timelineView.playheadColor, ComponentTheme.textPrimary)
    }

    function test_rulerModelGeneratesTicksInQml() {
        var root = createTemporaryObject(rulerModelComponent, this)
        verify(root !== null)

        var rulerModel = findChild(root, "rulerModel")
        verify(rulerModel !== null)
        tryVerify(function() { return rulerModel.count > 0 })

        var hasMajorLabel = false
        for (var i = 0; i < rulerModel.count; ++i) {
            var tick = rulerModel.tickAt(i)
            if (tick.isMajor && tick.label.length > 0) {
                hasMajorLabel = true
                break
            }
        }
        verify(hasMajorLabel)
    }

    function test_rulerPaintsVisiblePixels() {
        var ruler = createTemporaryObject(rulerPaintComponent, this)
        verify(ruler !== null)
        tryVerify(function() { return ruler.rulerModel.count > 0 })
        wait(100)

        var image = grabImage(ruler)
        var visiblePixels = 0
        for (var y = 0; y < image.height; ++y) {
            for (var x = 0; x < image.width; ++x) {
                if (image.alpha(x, y) > 0
                    && (image.red(x, y) !== 0
                        || image.green(x, y) !== 0
                        || image.blue(x, y) !== 0)) {
                    ++visiblePixels
                }
            }
        }
        verify(visiblePixels > 0)
    }

    function test_timelineViewEmbeddedRulerPaintsVisiblePixels() {
        ComponentTheme.style = ComponentTheme.Dark
        var timelineView = createTemporaryObject(directTimelineViewComponent, this)
        verify(timelineView !== null)
        timelineView.trackColor = "black"
        timelineView.rulerBg = "black"
        timelineView.rulerMajorColor = "red"
        timelineView.rulerMinorColor = "green"
        timelineView.rulerLabelColor = "blue"
        timelineView.rulerSeparatorColor = "yellow"

        tryVerify(function() { return timelineView.viewport.viewSpan > 0 })
        wait(100)

        var image = grabImage(timelineView)
        var visiblePixels = 0
        for (var y = 0; y < Math.min(timelineView.rulerHeight, image.height); ++y) {
            for (var x = 0; x < image.width; ++x) {
                var isTickPixel =
                    isNearColor(image, x, y, 255, 0, 0, 40)
                    || isNearColor(image, x, y, 0, 128, 0, 40)
                    || isNearColor(image, x, y, 255, 255, 0, 40)
                if (isTickPixel)
                    ++visiblePixels
            }
        }
        verify(visiblePixels > 0)
    }

    function test_timelineViewUsesTrackColorBehindTransparentRuler() {
        ComponentTheme.style = ComponentTheme.Dark
        var timelineView = createTemporaryObject(directTimelineViewComponent, this)
        verify(timelineView !== null)
        timelineView.trackColor = "black"
        timelineView.rulerBg = "transparent"
        timelineView.rulerMajorColor = "transparent"
        timelineView.rulerMinorColor = "transparent"
        timelineView.rulerLabelColor = "transparent"
        timelineView.rulerSeparatorColor = "transparent"

        tryVerify(function() { return timelineView.viewport.viewSpan > 0 })
        wait(100)

        var image = grabImage(timelineView)
        var blackPixels = 0
        var rulerPixelCount = Math.min(timelineView.rulerHeight, image.height) * image.width
        for (var y = 0; y < Math.min(timelineView.rulerHeight, image.height); ++y) {
            for (var x = 0; x < image.width; ++x) {
                if (isNearColor(image, x, y, 0, 0, 0, 8))
                    ++blackPixels
            }
        }
        verify(blackPixels > rulerPixelCount * 0.9)
    }

    function test_timelineViewRulerVisibleWithDarkTheme() {
        verifyTimelineViewRulerVisibleWithTheme(ComponentTheme.Dark)
    }

    function test_timelineViewRulerVisibleWithLightTheme() {
        verifyTimelineViewRulerVisibleWithTheme(ComponentTheme.Light)
    }

    function test_timelineViewRulerHasVisibleBackgroundBandWithDarkTheme() {
        verifyTimelineViewRulerBackgroundBand(ComponentTheme.Dark)
    }

    function verifyTimelineViewRulerVisibleWithTheme(style) {
        ComponentTheme.style = style
        var root = createTemporaryObject(themedSurfaceTimelineViewComponent, this)
        verify(root !== null)

        var timelineView = findChild(root, "timelineView")
        verify(timelineView !== null)

        tryVerify(function() { return timelineView.viewport.viewSpan > 0 })
        wait(100)

        var image = grabImage(root)
        var bgRed = Math.round(root.color.r * 255)
        var bgGreen = Math.round(root.color.g * 255)
        var bgBlue = Math.round(root.color.b * 255)
        var visiblePixels = 0
        for (var y = 0; y < Math.min(timelineView.rulerHeight, image.height); ++y) {
            for (var x = 0; x < image.width; ++x) {
                if (image.alpha(x, y) === 0)
                    continue

                var distance = Math.abs(image.red(x, y) - bgRed)
                    + Math.abs(image.green(x, y) - bgGreen)
                    + Math.abs(image.blue(x, y) - bgBlue)
                if (distance > 30)
                    ++visiblePixels
            }
        }
        verify(visiblePixels > 0)
    }

    function test_timelineViewRulerHasVisibleBackgroundBandWithLightTheme() {
        verifyTimelineViewRulerBackgroundBand(ComponentTheme.Light)
    }

    function verifyTimelineViewRulerBackgroundBand(style) {
        ComponentTheme.style = style
        var root = createTemporaryObject(themedSurfaceTimelineViewComponent, this)
        verify(root !== null)

        var timelineView = findChild(root, "timelineView")
        verify(timelineView !== null)

        tryVerify(function() { return timelineView.viewport.viewSpan > 0 })
        wait(100)

        var image = grabImage(root)
        var bgRed = Math.round(root.color.r * 255)
        var bgGreen = Math.round(root.color.g * 255)
        var bgBlue = Math.round(root.color.b * 255)
        var bandPixels = 0
        var rulerPixelCount = Math.min(timelineView.rulerHeight, image.height) * image.width
        for (var y = 0; y < Math.min(timelineView.rulerHeight, image.height); ++y) {
            for (var x = 0; x < image.width; ++x) {
                var distance = Math.abs(image.red(x, y) - bgRed)
                    + Math.abs(image.green(x, y) - bgGreen)
                    + Math.abs(image.blue(x, y) - bgBlue)
                if (distance > 20)
                    ++bandPixels
            }
        }
        verify(bandPixels > rulerPixelCount * 0.8)
    }

    function isNearColor(image, x, y, red, green, blue, tolerance) {
        if (image.alpha(x, y) === 0)
            return false

        return Math.abs(image.red(x, y) - red)
            + Math.abs(image.green(x, y) - green)
            + Math.abs(image.blue(x, y) - blue) <= tolerance
    }
}

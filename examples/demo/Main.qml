import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QuickUI.Components 1.0

ApplicationWindow {
    id: window
    width: 1000
    height: 680
    visible: true
    title: "QtQuickComponents Demo"

    color: ComponentTheme.style === ComponentTheme.Dark ? "#12131a" : "#ffffff"

    TimelineModel {
        id: timelineModel
    }

    Timer {
        id: playbackTimer
        interval: 120
        repeat: true
        running: playToggle.checked
        onTriggered: {
            var next = timeline.currentTime + 120000
            timeline.currentTime = next > timelineModel.totalEnd
                ? timelineModel.totalStart
                : next
        }
    }

    Component.onCompleted: {
        ComponentTheme.style = ComponentTheme.Dark
        timelineModel.addSegment(0, 900000, TimelineEnums.SegmentNormal)
        timelineModel.addSegment(1200000, 2300000, TimelineEnums.SegmentMotion)
        timelineModel.addSegment(2600000, 3500000, TimelineEnums.SegmentNormal)
        timelineModel.addSegment(3800000, 4300000, TimelineEnums.SegmentAlarm)
        timelineModel.addSegment(4700000, 6200000, TimelineEnums.SegmentNormal)
        timeline.currentTime = 0
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    role: "heading"
                    text: "QtQuickComponents"
                }

                Label {
                    role: "caption"
                    text: "Demo app for manual QML import and visual behavior checks"
                }
            }

            IconButton {
                iconText: ComponentTheme.style === ComponentTheme.Dark ? "☀" : "☾"
                tooltip: "Toggle theme"
                onClicked: {
                    ComponentTheme.style = ComponentTheme.style === ComponentTheme.Dark
                        ? ComponentTheme.Light
                        : ComponentTheme.Dark
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            radius: 8
            color: ComponentTheme.surface
            border.color: ComponentTheme.separator

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        role: "label"
                        text: "Controls"
                    }

                    RowLayout {
                        spacing: 10

                        Button {
                            text: "Filled"
                            onClicked: statusLabel.text = "Filled button clicked"
                        }

                        Button {
                            text: "Outline"
                            variant: "outline"
                            onClicked: statusLabel.text = "Outline button clicked"
                        }

                        Button {
                            text: "Ghost"
                            variant: "ghost"
                            onClicked: statusLabel.text = "Ghost button clicked"
                        }

                        Button {
                            text: "Loading"
                            loading: true
                        }
                    }

                    ProgressSlider {
                        Layout.fillWidth: true
                        value: timelineModel.totalEnd > 0
                            ? timeline.currentTime / timelineModel.totalEnd
                            : 0
                        bufferPosition: 0.72
                        onMoved: timeline.currentTime = value * timelineModel.totalEnd
                    }
                }

                TextField {
                    Layout.preferredWidth: 260
                    label: "Sample input"
                    placeholder: "Type to test focus"
                    clearable: true
                    onTextChanged: statusLabel.text = text.length > 0
                        ? "Input: " + text
                        : "Input cleared"
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: ComponentTheme.surface
            border.color: ComponentTheme.separator

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        Layout.fillWidth: true
                        role: "label"
                        text: "TimelineView"
                    }

                    Button {
                        id: playToggle
                        checkable: true
                        text: checked ? "Pause" : "Play"
                        variant: checked ? "outline" : "filled"
                    }

                    Button {
                        text: "Fit"
                        variant: "outline"
                        onClicked: timeline.fitAll()
                    }

                    Button {
                        text: "Center"
                        variant: "ghost"
                        onClicked: timeline.centerOnTime(timeline.currentTime)
                    }
                }

                TimelineView {
                    id: timeline
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: timelineModel
                    followMode: TimelineEnums.FollowEdge
                    segmentColors: ["#4c6ef5", "#20c997", "#ff6b6b"]
                    trackColor: ComponentTheme.style === ComponentTheme.Dark
                        ? "#141824"
                        : "#f4f6fb"
                    onSeeked: function(t) {
                        currentTime = t
                        statusLabel.text = "Seeked to " + Math.round(t / 1000) + "s"
                    }
                    onPlayRequested: function(t) {
                        currentTime = t
                        playToggle.checked = true
                        statusLabel.text = "Play requested at " + Math.round(t / 1000) + "s"
                    }
                }
            }
        }

        Label {
            id: statusLabel
            Layout.fillWidth: true
            role: "caption"
            text: "Use the controls, drag or wheel the timeline, and double-click to request playback."
        }
    }
}

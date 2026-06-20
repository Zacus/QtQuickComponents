import QtQuick
import QuickUI.Components 1.0

Rectangle {
    id: root

    property bool isActive: false
    property int alarmLevel: WndViewModel.None
    property color activeColor: "#2f80ed"
    property color warningColor: "#f2c94c"
    property color criticalColor: "#eb5757"
    readonly property bool hasBorder: alarmLevel !== WndViewModel.None || isActive

    color: "transparent"
    visible: hasBorder
    border.width: border.color.a > 0 ? 2 : 0
    border.color: alarmLevel === WndViewModel.Critical
        ? criticalColor
        : alarmLevel === WndViewModel.Warning
            ? warningColor
            : isActive ? activeColor : "transparent"
    opacity: alarmLevel === WndViewModel.Critical ? 0.35 : 1.0

    SequentialAnimation on opacity {
        running: root.alarmLevel === WndViewModel.Critical
        loops: Animation.Infinite
        NumberAnimation { from: 0.35; to: 1.0; duration: 420 }
        NumberAnimation { from: 1.0; to: 0.35; duration: 420 }
    }
}

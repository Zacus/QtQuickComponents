import QtQuick
import QuickUI.Components 1.0

Rectangle {
    id: root

    property bool isActive: false
    property int alarmLevel: WndViewModel.None
    property color activeColor: "#2f80ed"
    property color warningColor: "#f2c94c"
    property color criticalColor: "#eb5757"

    color: "transparent"
    border.width: border.color.a > 0 ? 2 : 0
    border.color: alarmLevel === 2
        ? criticalColor
        : alarmLevel === 1
            ? warningColor
            : isActive ? activeColor : "transparent"
    opacity: alarmLevel === 2 ? 0.35 : 1.0

    SequentialAnimation on opacity {
        running: root.alarmLevel === 2
        loops: Animation.Infinite
        NumberAnimation { from: 0.35; to: 1.0; duration: 420 }
        NumberAnimation { from: 1.0; to: 0.35; duration: 420 }
    }
}

import QtQuick
import QuickUI.Components 1.0

Item {
    id: root

    property var osdModel: []
    property real margin: 8

    function entryAt(position) {
        if (!osdModel)
            return null

        for (var i = 0; i < osdModel.length; ++i) {
            var entry = osdModel[i]
            if (entry && entry.position === position)
                return entry
        }
        return null
    }

    function textAt(position) {
        var entry = entryAt(position)
        return entry ? entry.text : ""
    }

    function colorAt(position) {
        var entry = entryAt(position)
        return entry && entry.color ? entry.color : "#ffffff"
    }

    Text {
        objectName: "osdTopLeftText"
        anchors { left: parent.left; top: parent.top; margins: root.margin }
        width: Math.max(0, parent.width / 2 - root.margin * 2)
        elide: Text.ElideRight
        text: root.textAt(0)
        color: root.colorAt(0)
        font.pixelSize: ComponentTheme.fontSizeCaption
        visible: text.length > 0
    }

    Text {
        objectName: "osdTopRightText"
        anchors { right: parent.right; top: parent.top; margins: root.margin }
        width: Math.max(0, parent.width / 2 - root.margin * 2)
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideRight
        text: root.textAt(1)
        color: root.colorAt(1)
        font.pixelSize: ComponentTheme.fontSizeCaption
        visible: text.length > 0
    }

    Text {
        objectName: "osdBottomLeftText"
        anchors { left: parent.left; bottom: parent.bottom; margins: root.margin }
        width: Math.max(0, parent.width / 2 - root.margin * 2)
        elide: Text.ElideRight
        text: root.textAt(2)
        color: root.colorAt(2)
        font.pixelSize: ComponentTheme.fontSizeCaption
        visible: text.length > 0
    }

    Text {
        objectName: "osdBottomRightText"
        anchors { right: parent.right; bottom: parent.bottom; margins: root.margin }
        width: Math.max(0, parent.width / 2 - root.margin * 2)
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideRight
        text: root.textAt(3)
        color: root.colorAt(3)
        font.pixelSize: ComponentTheme.fontSizeCaption
        visible: text.length > 0
    }
}

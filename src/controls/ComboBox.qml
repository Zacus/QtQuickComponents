import QtQuick
import QtQuick.Controls.Basic as QQC2
import QuickUI.Components 1.0

// 通用下拉选择框，基于 Qt Quick Controls ComboBox 保留键盘、焦点和弹出层语义。
QQC2.ComboBox {
    id: root

    property int delegateHeight: 30
    property int popupMaxVisibleItems: 8
    property string placeholderText: ""
    property color pressedBackgroundColor: ComponentTheme.buttonHover
    property color selectedBackgroundColor: ComponentTheme.surfaceHover
    property color selectedTextColor: ComponentTheme.textPrimary

    implicitWidth: Math.max(92, contentText.implicitWidth + leftPadding + rightPadding)
    implicitHeight: ComponentTheme.buttonSize
    leftPadding: 10
    rightPadding: 28
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true

    font.family: ComponentTheme.fontFamily
    font.pixelSize: ComponentTheme.fontSizeLabel
    font.weight: ComponentTheme.fontWeightMedium

    delegate: QQC2.ItemDelegate {
        id: option

        required property int index

        readonly property bool selected: root.currentIndex === index

        width: ListView.view ? ListView.view.width : root.width
        height: root.delegateHeight
        hoverEnabled: true
        highlighted: root.highlightedIndex === index

        contentItem: Text {
            text: root.textAt(option.index)
            font.family: ComponentTheme.fontFamily
            font.pixelSize: ComponentTheme.fontSizeLabel
            font.weight: option.selected || option.highlighted
                ? ComponentTheme.fontWeightMedium
                : ComponentTheme.fontWeightNormal
            color: option.selected || option.highlighted || option.hovered
                ? root.selectedTextColor
                : ComponentTheme.textSecondary
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        Rectangle {
            width: 2
            height: parent.height - 10
            radius: 1
            anchors.left: parent.left
            anchors.leftMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            color: ComponentTheme.accent
            visible: option.selected
        }

        background: Rectangle {
            radius: Math.max(4, ComponentTheme.buttonRadius - 2)
            color: {
                if (option.down) return root.pressedBackgroundColor
                if (option.selected) return root.selectedBackgroundColor
                if (option.highlighted || option.hovered) return ComponentTheme.buttonHover
                return "transparent"
            }

            Behavior on color { ColorAnimation { duration: ComponentTheme.durationFast } }
        }
    }

    indicator: Text {
        x: root.width - width - 10
        y: (root.height - height) / 2
        text: root.down ? "▴" : "▾"
        font.family: ComponentTheme.fontFamily
        font.pixelSize: 10
        color: root.enabled ? ComponentTheme.textSecondary : ComponentTheme.textDisabled
    }

    contentItem: Text {
        id: contentText

        leftPadding: 0
        rightPadding: 0
        text: root.displayText.length > 0 ? root.displayText : root.placeholderText
        font: root.font
        color: {
            if (!root.enabled) return ComponentTheme.textDisabled
            if (root.displayText.length === 0) return ComponentTheme.textDisabled
            return ComponentTheme.textPrimary
        }
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: ComponentTheme.buttonRadius
        color: {
            if (!root.enabled) return ComponentTheme.inputBg
            if (root.down) return root.pressedBackgroundColor
            if (root.hovered || root.activeFocus) return ComponentTheme.buttonHover
            return ComponentTheme.inputBg
        }
        border.width: root.down || root.activeFocus ? 1.5 : 1
        border.color: {
            if (!root.enabled) return ComponentTheme.separator
            if (root.down || root.activeFocus) return ComponentTheme.accent
            if (root.hovered) return ComponentTheme.inputBorder
            return ComponentTheme.separator
        }
        opacity: root.enabled ? 1.0 : 0.5

        Rectangle {
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 1 }
            height: 1
            radius: parent.radius
            color: Qt.rgba(1, 1, 1, root.down ? 0.05 : 0.14)
        }

        Rectangle {
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 1 }
            height: 1
            radius: parent.radius
            color: Qt.rgba(0, 0, 0, root.down ? 0.22 : 0.12)
        }

        Behavior on color { ColorAnimation { duration: ComponentTheme.durationFast } }
        Behavior on border.color { ColorAnimation { duration: ComponentTheme.durationFast } }
        Behavior on opacity { NumberAnimation { duration: ComponentTheme.durationFast } }
    }

    popup: QQC2.Popup {
        y: root.height + 6
        width: root.width
        padding: 4
        implicitHeight: Math.min(
            contentItem.implicitHeight + topPadding + bottomPadding,
            Math.max(1, root.popupMaxVisibleItems) * root.delegateHeight + topPadding + bottomPadding)

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds

            QQC2.ScrollIndicator.vertical: QQC2.ScrollIndicator {}
        }

        background: Rectangle {
            radius: ComponentTheme.buttonRadius
            color: ComponentTheme.surface
            border.width: 1
            border.color: ComponentTheme.separator

            Rectangle {
                anchors { left: parent.left; right: parent.right; top: parent.top; margins: 1 }
                height: 1
                radius: parent.radius
                color: Qt.rgba(1, 1, 1, 0.12)
            }
        }
    }

}

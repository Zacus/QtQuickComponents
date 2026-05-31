import QtQuick
import QtQuick.Controls.Basic as QQC2
import QuickUI.Components 1.0

// 单行文本输入框，基于 Qt Quick Controls TextField 提供输入、焦点和状态语义。
//
// 兼容属性：
//   label       — 输入框上方标签（空串则不占位）
//   placeholder — 占位提示文字，同时映射到 placeholderText
//   errorText   — 错误提示（非空时输入框变红描边）
//   password    — true 时隐藏输入内容（切换按钮可见）
//   clearable   — true 时有内容时显示清除按钮

QQC2.TextField {
    id: root

    property string label: ""
    property string placeholder: ""
    property string errorText: ""
    property bool password: false
    property bool clearable: false

    readonly property bool hasError: errorText.length > 0
    readonly property bool focused: activeFocus
    readonly property real _labelBlockHeight: label.length > 0
        ? labelText.implicitHeight + 4
        : 0
    readonly property real _errorBlockHeight: errorText.length > 0
        ? errorMsg.implicitHeight + 4
        : 0

    property bool _showPassword: false

    implicitWidth: 200
    implicitHeight: _labelBlockHeight + ComponentTheme.inputHeight + _errorBlockHeight

    placeholderText: placeholder
    echoMode: password && !_showPassword ? TextInput.Password : TextInput.Normal
    verticalAlignment: TextInput.AlignVCenter
    leftPadding: 10
    rightPadding: 10 + (clearable || password ? actionRow.implicitWidth + 10 : 0)
    topPadding: _labelBlockHeight
    bottomPadding: _errorBlockHeight

    font.family: ComponentTheme.fontFamily
    font.pixelSize: ComponentTheme.fontSize
    color: enabled ? ComponentTheme.inputText : ComponentTheme.textDisabled
    placeholderTextColor: ComponentTheme.inputPlaceholder
    selectionColor: Qt.rgba(
        ComponentTheme.accent.r,
        ComponentTheme.accent.g,
        ComponentTheme.accent.b,
        0.35)
    selectedTextColor: ComponentTheme.inputText

    onPasswordChanged: {
        if (!password)
            _showPassword = false
    }

    background: Item {
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            y: root._labelBlockHeight
            height: ComponentTheme.inputHeight
            radius: ComponentTheme.inputRadius

            color: root.enabled ? ComponentTheme.inputBg : ComponentTheme.surface
            opacity: root.enabled ? 1.0 : 0.6
            border.width: root.focused ? 1.5 : 1
            border.color: {
                if (!root.enabled) return ComponentTheme.inputBorder
                if (root.hasError) return "#e05555"
                if (root.focused) return ComponentTheme.inputFocus
                return ComponentTheme.inputBorder
            }

            Behavior on border.color { ColorAnimation { duration: ComponentTheme.durationFast } }
            Behavior on opacity { NumberAnimation { duration: ComponentTheme.durationFast } }
        }
    }

    Text {
        id: labelText
        visible: root.label.length > 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        text: root.label
        font.family: ComponentTheme.fontFamily
        font.pixelSize: ComponentTheme.fontSizeLabel
        font.weight: ComponentTheme.fontWeightMedium
        color: root.enabled ? ComponentTheme.textSecondary : ComponentTheme.textDisabled
    }

    Row {
        id: actionRow
        anchors.right: parent.right
        anchors.rightMargin: 6
        y: root._labelBlockHeight + (ComponentTheme.inputHeight - height) / 2
        spacing: 2
        visible: root.enabled && (clearButton.visible || passwordButton.visible)

        Rectangle {
            id: clearButton
            visible: root.clearable && root.text.length > 0
            width: 20
            height: 20
            radius: 10
            color: clearMouse.containsMouse ? ComponentTheme.surfaceHover : "transparent"

            Text {
                anchors.centerIn: parent
                text: "✕"
                font.pixelSize: 11
                color: ComponentTheme.textSecondary
            }

            MouseArea {
                id: clearMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.clear()
                    root.forceActiveFocus()
                }
            }
        }

        Rectangle {
            id: passwordButton
            visible: root.password
            width: 20
            height: 20
            radius: 10
            color: eyeMouse.containsMouse ? ComponentTheme.surfaceHover : "transparent"

            Text {
                anchors.centerIn: parent
                text: root._showPassword ? "🙈" : "👁"
                font.pixelSize: 12
            }

            MouseArea {
                id: eyeMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root._showPassword = !root._showPassword
            }
        }
    }

    Text {
        id: errorMsg
        visible: root.hasError
        anchors.left: parent.left
        anchors.right: parent.right
        y: root._labelBlockHeight + ComponentTheme.inputHeight + 4
        text: root.errorText
        font.family: ComponentTheme.fontFamily
        font.pixelSize: ComponentTheme.fontSizeCaption
        color: "#e05555"
        wrapMode: Text.WordWrap
    }
}

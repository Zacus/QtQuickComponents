import QtQuick
import QtQuick.Controls.Basic
import QuickUI.Components 1.0

// 单行文本输入框，包含：
//   label       — 输入框上方标签（空串则不占位）
//   placeholder — 占位提示文字
//   errorText   — 错误提示（非空时输入框变红描边）
//   password    — true 时隐藏输入内容（切换按钮可见）
//   enabled     — false 时禁用输入并降低透明度
//   clearable   — true 时有内容时显示清除按钮
//
// 暴露 text 属性与 accepted 信号，用法与 QtQuick.Controls TextInput 类似：
//   TextField {
//       label: "邮箱"
//       placeholder: "you@example.com"
//       errorText: root.emailError
//       onAccepted: root.submit()
//   }

Item {
    id: root

    property string text:        ""
    property string label:       ""
    property string placeholder: ""
    property string errorText:   ""
    property bool   password:    false
    property bool   enabled:     true
    property bool   clearable:   false

    signal accepted()
    signal textChanged(string text)

    // 只读状态
    readonly property bool hasError: errorText.length > 0
    readonly property bool focused:  input.activeFocus

    implicitWidth:  200
    implicitHeight: _totalHeight()

    function _totalHeight() {
        var h = ComponentTheme.inputHeight;
        if (root.label.length > 0)     h += labelText.implicitHeight + 4;
        if (root.errorText.length > 0) h += errorMsg.implicitHeight + 4;
        return h;
    }

    // ── 外部标签 ─────────────────────────────────────────────
    Text {
        id: labelText
        visible: root.label.length > 0
        anchors { left: parent.left; right: parent.right; top: parent.top }

        text:           root.label
        font.family:    ComponentTheme.fontFamily
        font.pixelSize: ComponentTheme.fontSizeLabel
        font.weight:    ComponentTheme.fontWeightMedium
        color:          root.enabled
                        ? ComponentTheme.textSecondary
                        : ComponentTheme.textDisabled
    }

    // ── 输入框主体 ───────────────────────────────────────────
    Rectangle {
        id: box
        anchors {
            left:  parent.left
            right: parent.right
            top:   root.label.length > 0 ? labelText.bottom : parent.top
            topMargin: root.label.length > 0 ? 4 : 0
        }
        height: ComponentTheme.inputHeight
        radius: ComponentTheme.inputRadius

        color:        root.enabled ? ComponentTheme.inputBg : ComponentTheme.surface
        opacity:      root.enabled ? 1.0 : 0.6

        border.width: root.focused ? 1.5 : 1
        border.color: {
            if (!root.enabled)    return ComponentTheme.inputBorder;
            if (root.hasError)    return "#e05555";
            if (root.focused)     return ComponentTheme.inputFocus;
            return ComponentTheme.inputBorder;
        }

        Behavior on border.color { ColorAnimation { duration: ComponentTheme.durationFast } }
        Behavior on opacity      { NumberAnimation { duration: ComponentTheme.durationFast } }

        // 占位文字
        Text {
            anchors {
                left: parent.left; leftMargin: 10
                right: actionRow.left; rightMargin: 4
                verticalCenter: parent.verticalCenter
            }
            visible: input.text.length === 0 && !input.activeFocus
            text:           root.placeholder
            font.family:    ComponentTheme.fontFamily
            font.pixelSize: ComponentTheme.fontSize
            color:          ComponentTheme.inputPlaceholder
            elide: Text.ElideRight
        }

        // 真正的 TextInput
        TextInput {
            id: input
            anchors {
                left: parent.left; leftMargin: 10
                right: actionRow.left; rightMargin: 4
                verticalCenter: parent.verticalCenter
            }

            text:           root.text
            font.family:    ComponentTheme.fontFamily
            font.pixelSize: ComponentTheme.fontSize
            color:          root.enabled
                            ? ComponentTheme.inputText
                            : ComponentTheme.textDisabled
            selectionColor: Qt.rgba(
                                ComponentTheme.accent.r,
                                ComponentTheme.accent.g,
                                ComponentTheme.accent.b, 0.35)
            selectedTextColor: ComponentTheme.inputText

            echoMode:      root.password && !_showPassword
                           ? TextInput.Password
                           : TextInput.Normal
            enabled:       root.enabled
            clip:          true

            onTextChanged: {
                root.text = text;
                root.textChanged(text);
            }
            onAccepted: root.accepted()
        }

        // ── 右侧操作区（清除 / 密码切换）────────────────────
        Row {
            id: actionRow
            anchors { right: parent.right; rightMargin: 6; verticalCenter: parent.verticalCenter }
            spacing: 2

            // 清除按钮
            Rectangle {
                visible:      root.clearable && input.text.length > 0
                width: 20; height: 20; radius: 10
                color: clearMouse.containsMouse
                       ? ComponentTheme.surfaceHover : "transparent"

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
                    cursorShape:  Qt.PointingHandCursor
                    onClicked: { input.text = ""; input.forceActiveFocus(); }
                }
            }

            // 密码可见切换
            Rectangle {
                visible:      root.password
                width: 20; height: 20; radius: 10
                color: eyeMouse.containsMouse
                       ? ComponentTheme.surfaceHover : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: _showPassword ? "🙈" : "👁"
                    font.pixelSize: 12
                }
                MouseArea {
                    id: eyeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape:  Qt.PointingHandCursor
                    onClicked: _showPassword = !_showPassword
                }
            }
        }

        // 点击空白处也聚焦
        MouseArea {
            anchors.fill: parent
            onClicked:    input.forceActiveFocus()
            // 不拦截子控件事件
            enabled: !input.activeFocus
        }
    }

    // ── 错误提示 ─────────────────────────────────────────────
    Text {
        id: errorMsg
        visible: root.hasError
        anchors {
            left:  parent.left
            right: parent.right
            top:   box.bottom
            topMargin: 4
        }
        text:           root.errorText
        font.family:    ComponentTheme.fontFamily
        font.pixelSize: ComponentTheme.fontSizeCaption
        color:          "#e05555"
        wrapMode:       Text.WordWrap
    }

    // 内部状态：密码是否可见
    property bool _showPassword: false

    // 向外暴露聚焦方法
    function forceActiveFocus() { input.forceActiveFocus(); }
}

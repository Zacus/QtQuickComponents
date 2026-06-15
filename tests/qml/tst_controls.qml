import QtQuick
import QtTest
import QuickUI.Components 1.0

TestCase {
    name: "Controls"

    Component {
        id: textFieldComponent

        TextField {
            text: "initial"
            label: "Name"
            placeholder: "Full name"
            clearable: true
        }
    }

    Component {
        id: passwordFieldComponent

        TextField {
            text: "secret"
            password: true
        }
    }

    Component {
        id: buttonComponent

        Button {
            text: "Save"
            variant: "outline"
        }
    }

    Component {
        id: iconButtonComponent

        IconButton {
            iconText: "?"
            tooltip: "Help"
        }
    }

    Component {
        id: progressSliderComponent

        ProgressSlider {
            from: 0
            to: 100
            value: 25
            bufferPosition: 0.6
        }
    }

    Component {
        id: comboBoxComponent

        ComboBox {
            model: ListModel {
                ListElement { code: "en_US"; label: "EN" }
                ListElement { code: "zh_CN"; label: "中文" }
            }
            textRole: "label"
            valueRole: "code"
            currentIndex: 1
            popupMaxVisibleItems: 4
        }
    }

    Component {
        id: labelComponent

        Label {
            text: "Settings"
            role: "heading"
        }
    }

    function test_textFieldLoadsAndExposesText() {
        var field = createTemporaryObject(textFieldComponent, this)

        verify(field !== null)
        compare(field.text, "initial")
        compare(field.label, "Name")
        compare(field.placeholder, "Full name")
        compare(field.placeholderText, "Full name")
        compare(field.hovered, false)
        compare(field.clearable, true)
    }

    function test_textFieldPasswordMapsToEchoMode() {
        var field = createTemporaryObject(passwordFieldComponent, this)

        verify(field !== null)
        compare(field.password, true)
        compare(field.echoMode, TextInput.Password)
    }

    function test_buttonUsesControlStateApi() {
        var button = createTemporaryObject(buttonComponent, this)

        verify(button !== null)
        compare(button.text, "Save")
        compare(button.variant, "outline")
        compare(button.down, false)
        compare(button.hovered, false)
        compare(button.loading, false)
    }

    function test_iconButtonUsesControlStateApi() {
        var button = createTemporaryObject(iconButtonComponent, this)

        verify(button !== null)
        compare(button.iconText, "?")
        compare(button.tooltip, "Help")
        compare(button.down, false)
        compare(button.hovered, false)
        compare(button.enabled, true)
    }

    function test_progressSliderUsesSliderStateApi() {
        var slider = createTemporaryObject(progressSliderComponent, this)

        verify(slider !== null)
        compare(slider.from, 0)
        compare(slider.to, 100)
        compare(slider.value, 25)
        compare(slider.bufferPosition, 0.6)
        compare(slider.pressed, false)
        compare(slider.hovered, false)
        compare(slider.enabled, true)
    }

    function test_comboBoxUsesControlStateApi() {
        var combo = createTemporaryObject(comboBoxComponent, this)

        verify(combo !== null)
        compare(combo.currentIndex, 1)
        compare(combo.currentText, "中文")
        compare(combo.currentValue, "zh_CN")
        compare(combo.popupMaxVisibleItems, 4)
        compare(combo.pressedBackgroundColor, ComponentTheme.buttonHover)
        compare(combo.selectedBackgroundColor, ComponentTheme.surfaceHover)
        compare(combo.selectedTextColor, ComponentTheme.textPrimary)
        compare(combo.hovered, false)
        compare(combo.down, false)
    }

    function test_comboBoxDelegateUsesTextRole() {
        var combo = createTemporaryObject(comboBoxComponent, this)
        verify(combo !== null)

        var first = combo.delegate.createObject(combo, { "index": 0 })
        verify(first !== null)
        compare(first.contentItem.text, "EN")
        first.destroy()

        var second = combo.delegate.createObject(combo, { "index": 1 })
        verify(second !== null)
        compare(second.contentItem.text, "中文")
        second.destroy()
    }

    function test_labelUsesControlLabelApiAndRole() {
        var label = createTemporaryObject(labelComponent, this)

        verify(label !== null)
        compare(label.text, "Settings")
        compare(label.role, "heading")
        compare(label.horizontalAlignment, Text.AlignLeft)
        compare(label.font.weight, ComponentTheme.fontWeightBold)
    }
}

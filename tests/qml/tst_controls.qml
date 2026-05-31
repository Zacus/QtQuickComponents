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
            clearable: true
        }
    }

    Component {
        id: buttonComponent

        Button {
            text: "Save"
            variant: "outline"
        }
    }

    function test_textFieldLoadsAndExposesText() {
        var field = createTemporaryObject(textFieldComponent, this)

        verify(field !== null)
        compare(field.text, "initial")
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
}

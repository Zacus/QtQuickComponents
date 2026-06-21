qt_add_executable(qtc_demo
    main.cpp
)

qt_add_qml_module(qtc_demo
    URI QtQuickComponentsDemo
    VERSION 1.0
    IMPORT_PATH
        "${QTC_QML_OUTPUT_BASE}"
    DEPENDENCIES
        QtQuick
        QtQuick.Controls.Basic
        QtQuick.Layouts
        QuickUI.Components
        QuickUI.Components.impl
    QML_FILES Main.qml
)

target_link_libraries(qtc_demo PRIVATE
    QtQuickComponents
    QtQuickComponentsImpl
    Qt6::Quick
    Qt6::QuickControls2
)

target_compile_definitions(qtc_demo PRIVATE
    QTC_DEMO_QML_IMPORT_PATH="${QTC_QML_OUTPUT_BASE}"
)

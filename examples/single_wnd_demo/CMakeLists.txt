qt_add_executable(qtc_single_wnd_demo
    main.cpp
)
target_include_directories(qtc_single_wnd_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/src/window
)

qt_add_qml_module(qtc_single_wnd_demo
    URI SingleWndDemo
    VERSION 1.0
    IMPORT_PATH
        "${QTC_QML_OUTPUT_BASE}"
    DEPENDENCIES
        QtQuick
        QtQuick.Controls.Basic
        QtQuick.Layouts
        QuickUI.Components
        QuickUI.Components.impl
    SOURCES
        FramePump.h
        FramePump.cpp
    QML_FILES
        Main.qml
)

target_link_libraries(qtc_single_wnd_demo PRIVATE
    QtQuickComponents
    QtQuickComponentsImpl
    Qt6::Quick
    Qt6::QuickControls2
)

target_compile_definitions(qtc_single_wnd_demo PRIVATE
    QTC_DEMO_QML_IMPORT_PATH="${QTC_QML_OUTPUT_BASE}"
)

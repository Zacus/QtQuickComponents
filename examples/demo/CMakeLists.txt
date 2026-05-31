qt_add_executable(qtc_demo
    main.cpp
)

qt_add_qml_module(qtc_demo
    URI QtQuickComponentsDemo
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(qtc_demo PRIVATE
    QtQuickComponents
    Qt6::Quick
)

target_compile_definitions(qtc_demo PRIVATE
    QTC_DEMO_QML_IMPORT_PATH="${QTC_QML_OUTPUT_BASE}"
)

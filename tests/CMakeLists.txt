find_package(Qt6 REQUIRED COMPONENTS QuickTest Test)

qt_add_executable(tst_timeline_model
    tst_timeline_model.cpp
)

target_link_libraries(tst_timeline_model PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME TimelineModel COMMAND tst_timeline_model)

qt_add_executable(tst_timeline_viewport
    tst_timeline_viewport.cpp
)

target_link_libraries(tst_timeline_viewport PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME TimelineViewport COMMAND tst_timeline_viewport)

qt_add_executable(tst_ruler_model
    tst_ruler_model.cpp
)

target_link_libraries(tst_ruler_model PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME RulerModel COMMAND tst_ruler_model)

qt_add_executable(tst_component_theme
    tst_component_theme.cpp
)

target_link_libraries(tst_component_theme PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME ComponentTheme COMMAND tst_component_theme)

qt_add_executable(tst_theme_json_loader
    tst_theme_json_loader.cpp
)

target_link_libraries(tst_theme_json_loader PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME ThemeJsonLoader COMMAND tst_theme_json_loader)

qt_add_executable(tst_qml_components
    tst_qml_components.cpp
)

target_link_libraries(tst_qml_components PRIVATE
    QtQuickComponents
    Qt6::QuickTest
)

add_test(
    NAME QmlComponents
    COMMAND tst_qml_components -input "${CMAKE_CURRENT_SOURCE_DIR}/qml"
)
set_tests_properties(QmlComponents PROPERTIES
    ENVIRONMENT "QML2_IMPORT_PATH=${CMAKE_BINARY_DIR};QML_IMPORT_PATH=${CMAKE_BINARY_DIR};QT_QPA_PLATFORM=offscreen"
)

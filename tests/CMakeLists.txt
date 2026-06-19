find_package(Qt6 REQUIRED COMPONENTS Quick QuickTest Test)

set(QTC_TEST_PRIVATE_INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/src/theme"
    "${PROJECT_SOURCE_DIR}/src/timeline"
    "${PROJECT_SOURCE_DIR}/src/monitor"
)

qt_add_executable(tst_timeline_model
    tst_timeline_model.cpp
)

target_link_libraries(tst_timeline_model PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME TimelineModel COMMAND tst_timeline_model)

qt_add_executable(tst_wnd_view_model
    tst_wnd_view_model.cpp
)

target_link_libraries(tst_wnd_view_model PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME WndViewModel COMMAND tst_wnd_view_model)

qt_add_executable(tst_global_video_renderer
    tst_global_video_renderer.cpp
)
target_include_directories(tst_global_video_renderer PRIVATE
    ${QTC_TEST_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(tst_global_video_renderer PRIVATE
    QtQuickComponentsImpl
    Qt6::Quick
    Qt6::Test
)

add_test(NAME GlobalVideoRenderer COMMAND tst_global_video_renderer)

qt_add_executable(tst_video_surface
    tst_video_surface.cpp
)
target_include_directories(tst_video_surface PRIVATE
    ${QTC_TEST_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(tst_video_surface PRIVATE
    QtQuickComponentsImpl
    Qt6::Quick
    Qt6::Test
)

add_test(NAME VideoSurface COMMAND tst_video_surface)
set_tests_properties(VideoSurface PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

qt_add_executable(tst_timeline_viewport
    tst_timeline_viewport.cpp
)
target_include_directories(tst_timeline_viewport PRIVATE
    ${QTC_TEST_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(tst_timeline_viewport PRIVATE
    QtQuickComponentsImpl
    Qt6::Quick
    Qt6::Test
)

add_test(NAME TimelineViewport COMMAND tst_timeline_viewport)

qt_add_executable(tst_ruler_model
    tst_ruler_model.cpp
)
target_include_directories(tst_ruler_model PRIVATE
    ${QTC_TEST_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(tst_ruler_model PRIVATE
    QtQuickComponentsImpl
    Qt6::Quick
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
target_include_directories(tst_theme_json_loader PRIVATE
    ${QTC_TEST_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(tst_theme_json_loader PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME ThemeJsonLoader COMMAND tst_theme_json_loader)

qt_add_executable(tst_theme_file_watcher
    tst_theme_file_watcher.cpp
)
target_include_directories(tst_theme_file_watcher PRIVATE
    ${QTC_TEST_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(tst_theme_file_watcher PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME ThemeFileWatcher COMMAND tst_theme_file_watcher)

add_test(
    NAME PublicHeaders
    COMMAND ${CMAKE_COMMAND}
        -D QTC_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_public_headers.cmake"
)

add_test(
    NAME QmlApiSurface
    COMMAND ${CMAKE_COMMAND}
        -D QTC_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -D QTC_QMLDIR_FILE=$<TARGET_PROPERTY:QtQuickComponents,QT_QML_MODULE_OUTPUT_DIRECTORY>/qmldir
        -D QTC_IMPL_QMLDIR_FILE=$<TARGET_PROPERTY:QtQuickComponentsImpl,QT_QML_MODULE_OUTPUT_DIRECTORY>/qmldir
        -D QTC_QMLTYPES_FILE=$<TARGET_PROPERTY:QtQuickComponents,QT_QML_MODULE_OUTPUT_DIRECTORY>/QtQuickComponents.qmltypes
        -D QTC_IMPL_QMLTYPES_FILE=$<TARGET_PROPERTY:QtQuickComponentsImpl,QT_QML_MODULE_OUTPUT_DIRECTORY>/QtQuickComponentsImpl.qmltypes
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_qml_api_surface.cmake"
)

qt_add_executable(tst_qml_components
    tst_qml_components.cpp
)

target_link_libraries(tst_qml_components PRIVATE
    QtQuickComponents
    QtQuickComponentsImpl
    Qt6::QuickTest
)

add_test(
    NAME QmlComponents
    COMMAND tst_qml_components -input "${CMAKE_CURRENT_SOURCE_DIR}/qml"
)
set_tests_properties(QmlComponents PROPERTIES
    ENVIRONMENT "QML2_IMPORT_PATH=${CMAKE_BINARY_DIR};QML_IMPORT_PATH=${CMAKE_BINARY_DIR};QT_QPA_PLATFORM=offscreen"
)

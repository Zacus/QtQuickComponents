cmake_minimum_required(VERSION 3.21)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" QTC_VERSION)
string(STRIP "${QTC_VERSION}" QTC_VERSION)
if(NOT QTC_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION.txt must use MAJOR.MINOR.PATCH format, got: ${QTC_VERSION}")
endif()

project(QtQuickComponents
    VERSION     ${QTC_VERSION}
    DESCRIPTION "Reusable Qt Quick UI component library"
    LANGUAGES   CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 GuiPrivate ShaderTools)
qt_standard_project_setup()
include(GNUInstallDirs)

option(QTC_BUILD_TESTS "Build QtQuickComponents tests" ${PROJECT_IS_TOP_LEVEL})
option(QTC_BUILD_EXAMPLES "Build QtQuickComponents example applications" OFF)

# ── 检测 Qt 构建类型 ──────────────────────────────────────────
# Qt 静态构建时 QT6_IS_SHARED_LIBS_BUILD 为 FALSE
get_target_property(_qt_core_type Qt6::Core TYPE)
if(_qt_core_type STREQUAL "STATIC_LIBRARY")
    set(_QTC_STATIC_QT TRUE)
else()
    set(_QTC_STATIC_QT FALSE)
endif()

# ── 库类型随 Qt 构建类型切换 ──────────────────────────────────
# 动态 Qt：SHARED，插件编译成 .dylib，运行时加载
# 静态 Qt：STATIC，插件静态链入可执行文件，Qt 自动生成 Q_IMPORT_PLUGIN
if(_QTC_STATIC_QT)
    qt_add_library(QtQuickComponents STATIC)
    qt_add_library(QtQuickComponentsImpl STATIC)
else()
    qt_add_library(QtQuickComponents SHARED)
    qt_add_library(QtQuickComponentsImpl SHARED)
endif()

# OUTPUT_DIRECTORY 由调用方通过 QTC_QML_OUTPUT_BASE 传入
if(NOT DEFINED QTC_QML_OUTPUT_BASE)
    set(QTC_QML_OUTPUT_BASE "${CMAKE_BINARY_DIR}")
endif()

# QML 模块路径（在 OUTPUT_DIRECTORY 和 install 规则中使用）
set(QTC_QML_MODULE_PATH "${QTC_QML_OUTPUT_BASE}/QuickUI/Components")
set(QTC_QML_IMPL_MODULE_PATH "${QTC_QML_MODULE_PATH}/impl")

set(QTC_PUBLIC_HEADERS
    include/QtQuickComponents/ComponentTheme.h
    include/QtQuickComponents/TimelineEnums.h
    include/QtQuickComponents/TimelineSegment.h
    include/QtQuickComponents/TimelineModel.h
    include/QtQuickComponents/WndViewModel.h
)

set(QTC_PRIVATE_HEADERS
    src/theme/ThemeFileWatcher.h
    src/theme/ThemeJsonLoader.h
    src/theme/ThemeTokens.h
)

set(QTC_IMPL_HEADERS
    src/timeline/TimelineViewport.h
    src/timeline/RulerTick.h
    src/timeline/RulerModel.h
    src/timeline/TimelineTrackModel.h
    src/monitor/GlobalVideoRenderer.h
    src/monitor/VideoSurface.h
    src/monitor/Yuv420GeometryBuffer.h
    src/monitor/Yuv420GraphicsPipeline.h
    src/monitor/Yuv420ShaderPipeline.h
    src/monitor/Yuv420ShaderUniforms.h
    src/monitor/Yuv420TextureSet.h
    src/monitor/Yuv420RenderNode.h
)

set(QTC_CPP_SOURCES
    src/theme/ComponentTheme.cpp
    src/theme/ThemeFileWatcher.cpp
    src/theme/ThemeJsonLoader.cpp
    src/timeline/TimelineModel.cpp
    src/monitor/WndViewModel.cpp
)

set(QTC_IMPL_CPP_SOURCES
    src/timeline/TimelineViewport.cpp
    src/timeline/RulerModel.cpp
    src/timeline/TimelineTrackModel.cpp
    src/monitor/GlobalVideoRenderer.cpp
    src/monitor/VideoSurface.cpp
    src/monitor/Yuv420GeometryBuffer.cpp
    src/monitor/Yuv420GraphicsPipeline.cpp
    src/monitor/Yuv420ShaderPipeline.cpp
    src/monitor/Yuv420ShaderUniforms.cpp
    src/monitor/Yuv420TextureSet.cpp
    src/monitor/Yuv420RenderNode.cpp
)

set(QTC_PUBLIC_QML_FILES
    src/controls/ComboBox.qml
    src/controls/IconButton.qml
    src/controls/ProgressSlider.qml
    src/controls/Button.qml
    src/controls/Label.qml
    src/controls/TextField.qml
    src/monitor/SingleWnd.qml
    src/timeline/TimelineView.qml
)

set(QTC_INTERNAL_QML_FILES
    src/monitor/NoSignalLayer.qml
    src/monitor/OSDLayer.qml
    src/monitor/BorderLayer.qml
    src/monitor/TitleBar.qml
    src/timeline/TimelineTrack.qml
    src/timeline/TimelineRuler.qml
    src/timeline/TimelineInputHandler.qml
    src/timeline/PlayheadOverlay.qml
)

set(QTC_QML_FILES
    ${QTC_PUBLIC_QML_FILES}
    ${QTC_INTERNAL_QML_FILES}
)

file(GLOB QTC_THEME_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/themes/*.json"
)
set(QTC_THEME_OUTPUT_DIR "${QTC_QML_OUTPUT_BASE}/themes")
foreach(_qtc_theme_file IN LISTS QTC_THEME_FILES)
    get_filename_component(_qtc_theme_name "${_qtc_theme_file}" NAME)
    set_source_files_properties("${_qtc_theme_file}" PROPERTIES
        QT_RESOURCE_ALIAS "themes/${_qtc_theme_name}"
    )
endforeach()

set_source_files_properties(
    src/controls/ComboBox.qml PROPERTIES QT_RESOURCE_ALIAS ComboBox.qml
)
set_source_files_properties(
    src/controls/IconButton.qml PROPERTIES QT_RESOURCE_ALIAS IconButton.qml
)
set_source_files_properties(
    src/controls/ProgressSlider.qml PROPERTIES QT_RESOURCE_ALIAS ProgressSlider.qml
)
set_source_files_properties(
    src/controls/Button.qml PROPERTIES QT_RESOURCE_ALIAS Button.qml
)
set_source_files_properties(
    src/controls/Label.qml PROPERTIES QT_RESOURCE_ALIAS Label.qml
)
set_source_files_properties(
    src/controls/TextField.qml PROPERTIES QT_RESOURCE_ALIAS TextField.qml
)
set_source_files_properties(
    src/monitor/SingleWnd.qml PROPERTIES QT_RESOURCE_ALIAS SingleWnd.qml
)
set_source_files_properties(
    src/monitor/NoSignalLayer.qml PROPERTIES QT_RESOURCE_ALIAS NoSignalLayer.qml
)
set_source_files_properties(
    src/monitor/OSDLayer.qml PROPERTIES QT_RESOURCE_ALIAS OSDLayer.qml
)
set_source_files_properties(
    src/monitor/BorderLayer.qml PROPERTIES QT_RESOURCE_ALIAS BorderLayer.qml
)
set_source_files_properties(
    src/monitor/TitleBar.qml PROPERTIES QT_RESOURCE_ALIAS TitleBar.qml
)
set_source_files_properties(
    src/timeline/TimelineTrack.qml PROPERTIES QT_RESOURCE_ALIAS TimelineTrack.qml
)
set_source_files_properties(
    src/timeline/TimelineRuler.qml PROPERTIES QT_RESOURCE_ALIAS TimelineRuler.qml
)
set_source_files_properties(
    src/timeline/TimelineInputHandler.qml PROPERTIES QT_RESOURCE_ALIAS TimelineInputHandler.qml
)
set_source_files_properties(
    src/timeline/PlayheadOverlay.qml PROPERTIES QT_RESOURCE_ALIAS PlayheadOverlay.qml
)
set_source_files_properties(
    src/timeline/TimelineView.qml PROPERTIES QT_RESOURCE_ALIAS TimelineView.qml
)

qt_add_qml_module(QtQuickComponentsImpl
    URI     "QuickUI.Components.impl"
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    OUTPUT_DIRECTORY "${QTC_QML_IMPL_MODULE_PATH}"
    SOURCES ${QTC_IMPL_HEADERS} ${QTC_IMPL_CPP_SOURCES}
    QML_FILES ${QTC_INTERNAL_QML_FILES}
)

qt_add_shaders(QtQuickComponentsImpl "qtc_yuv420_shaders"
    QUIET
    PREFIX "/QuickUI/Components/impl/shaders"
    FILES
        src/monitor/shaders/Yuv420Blit.vert
        src/monitor/shaders/Yuv420Blit.frag
    OUTPUTS
        Yuv420Blit.vert.qsb
        Yuv420Blit.frag.qsb
)

qt_add_qml_module(QtQuickComponents
    URI     "QuickUI.Components"
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    OUTPUT_DIRECTORY "${QTC_QML_MODULE_PATH}"
    SOURCES ${QTC_PUBLIC_HEADERS} ${QTC_PRIVATE_HEADERS} ${QTC_CPP_SOURCES}
    QML_FILES ${QTC_PUBLIC_QML_FILES}
    RESOURCES ${QTC_THEME_FILES}
)

add_custom_target(QtQuickComponents_copy_themes ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${QTC_THEME_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QTC_THEME_FILES} "${QTC_THEME_OUTPUT_DIR}"
    DEPENDS ${QTC_THEME_FILES}
    COMMENT "Copying QtQuickComponents theme JSON files"
)
add_dependencies(QtQuickComponents QtQuickComponents_copy_themes)

# 动态构建：qt_add_qml_module 创建的插件目标名为 QtQuickComponentsplugin（Qt 约定）
# 需要确保插件 .dylib/.so 与 qmldir 输出到同一目录，QML 运行时才能找到
if(NOT _QTC_STATIC_QT)
    set_target_properties(QtQuickComponentsImplplugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${QTC_QML_IMPL_MODULE_PATH}"
        RUNTIME_OUTPUT_DIRECTORY "${QTC_QML_IMPL_MODULE_PATH}"
    )
    set_target_properties(QtQuickComponentsplugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${QTC_QML_MODULE_PATH}"
        RUNTIME_OUTPUT_DIRECTORY "${QTC_QML_MODULE_PATH}"
    )
endif()

target_include_directories(QtQuickComponents PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/QtQuickComponents>
    $<INSTALL_INTERFACE:include/QtQuickComponents>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/theme
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timeline
    ${CMAKE_CURRENT_SOURCE_DIR}/src/monitor
)

target_include_directories(QtQuickComponentsImpl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/QtQuickComponents
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timeline
    ${CMAKE_CURRENT_SOURCE_DIR}/src/monitor
)

target_link_libraries(QtQuickComponents PUBLIC
    Qt6::Quick
    Qt6::QuickControls2
)

target_link_libraries(QtQuickComponentsImpl PRIVATE
    QtQuickComponents
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::GuiPrivate
)

target_compile_definitions(QtQuickComponents PRIVATE
    QTC_THEME_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/themes"
    QTC_THEME_BUILD_DIR="${QTC_THEME_OUTPUT_DIR}"
    QTC_THEME_INSTALL_DIR="${CMAKE_INSTALL_FULL_DATADIR}/QtQuickComponents/themes"
)
if(QTC_BUILD_TESTS)
    target_compile_definitions(QtQuickComponents PRIVATE QTC_ENABLE_THEME_TEST_OVERRIDES)
endif()

# ── 安装规则 ──────────────────────────────────────────────────
include(CMakePackageConfigHelpers)

install(TARGETS QtQuickComponents
    EXPORT QtQuickComponentsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS QtQuickComponentsImpl
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(NOT _QTC_STATIC_QT)
    install(TARGETS QtQuickComponentsImplplugin
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(TARGETS QtQuickComponentsplugin
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
install(FILES
    ${QTC_PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QtQuickComponents
)
install(FILES
    ${QTC_PUBLIC_QML_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/qml
)
install(DIRECTORY "${QTC_QML_OUTPUT_BASE}/QuickUI/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/QuickUI
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/themes/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/themes
    FILES_MATCHING PATTERN "*.json"
)

install(EXPORT QtQuickComponentsTargets
    FILE QtQuickComponentsTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtQuickComponents
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtQuickComponentsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QtQuickComponentsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtQuickComponents
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QtQuickComponentsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QtQuickComponentsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QtQuickComponentsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtQuickComponents
)

if(QTC_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

if(QTC_BUILD_EXAMPLES)
    add_subdirectory(examples/demo)
endif()

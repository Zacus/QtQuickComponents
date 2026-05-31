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

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup()

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
else()
    qt_add_library(QtQuickComponents SHARED)
endif()

# OUTPUT_DIRECTORY 由调用方通过 QTC_QML_OUTPUT_BASE 传入
if(NOT DEFINED QTC_QML_OUTPUT_BASE)
    set(QTC_QML_OUTPUT_BASE "${CMAKE_BINARY_DIR}")
endif()

# QML 模块路径（在 OUTPUT_DIRECTORY 和 install 规则中使用）
set(QTC_QML_MODULE_PATH "${QTC_QML_OUTPUT_BASE}/QuickUI/Components")

set(QTC_PUBLIC_HEADERS
    src/theme/ComponentTheme.h
    src/timeline/TimelineEnums.h
    src/timeline/TimelineSegment.h
    src/timeline/TimelineModel.h
    src/timeline/TimelineViewport.h
    src/timeline/RulerTick.h
    src/timeline/RulerModel.h
    src/timeline/TimelineTrackModel.h
)

set(QTC_CPP_SOURCES
    src/theme/ComponentTheme.cpp
    src/timeline/TimelineModel.cpp
    src/timeline/TimelineViewport.cpp
    src/timeline/RulerModel.cpp
    src/timeline/TimelineTrackModel.cpp
)

set(QTC_QML_FILES
    src/controls/IconButton.qml
    src/controls/ProgressSlider.qml
    src/controls/Button.qml
    src/controls/Label.qml
    src/controls/TextField.qml
    src/timeline/TimelineTrack.qml
    src/timeline/TimelineRuler.qml
    src/timeline/TimelineInputHandler.qml
    src/timeline/PlayheadOverlay.qml
    src/timeline/TimelineView.qml
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

qt_add_qml_module(QtQuickComponents
    URI     "QuickUI.Components"
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    OUTPUT_DIRECTORY "${QTC_QML_MODULE_PATH}"
    SOURCES ${QTC_PUBLIC_HEADERS} ${QTC_CPP_SOURCES}
    QML_FILES ${QTC_QML_FILES}
)

# 动态构建：qt_add_qml_module 创建的插件目标名为 QtQuickComponentsplugin（Qt 约定）
# 需要确保插件 .dylib/.so 与 qmldir 输出到同一目录，QML 运行时才能找到
if(NOT _QTC_STATIC_QT)
    set_target_properties(QtQuickComponentsplugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${QTC_QML_MODULE_PATH}"
        RUNTIME_OUTPUT_DIRECTORY "${QTC_QML_MODULE_PATH}"
    )
endif()

target_include_directories(QtQuickComponents PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/theme>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/timeline>
    $<INSTALL_INTERFACE:include/QtQuickComponents>
)

target_link_libraries(QtQuickComponents PUBLIC
    Qt6::Quick
    Qt6::QuickControls2
)

# ── 安装规则 ──────────────────────────────────────────────────
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS QtQuickComponents
    EXPORT QtQuickComponentsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(NOT _QTC_STATIC_QT)
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
    ${QTC_QML_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/qml
)
install(DIRECTORY "${QTC_QML_OUTPUT_BASE}/QuickUI/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/QuickUI
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

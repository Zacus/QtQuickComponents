cmake_minimum_required(VERSION 3.21)

project(QtQuickComponents
    VERSION     1.0.0
    DESCRIPTION "Reusable Qt Quick UI component library"
    LANGUAGES   CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup()

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

qt_add_qml_module(QtQuickComponents
    URI     "QuickUI.Components"
    VERSION 1.0
    OUTPUT_DIRECTORY "${QTC_QML_MODULE_PATH}"
    SOURCES
        src/ComponentTheme.h
        src/ComponentTheme.cpp
        src/TimelineSegment.h
        src/TimelineModel.h
        src/TimelineModel.cpp
        src/TimelineViewport.h
        src/TimelineViewport.cpp
        src/RulerTick.h
        src/RulerModel.h
        src/RulerModel.cpp
    QML_FILES
        qml/IconButton.qml
        qml/ProgressSlider.qml
        qml/Button.qml
        qml/Label.qml
        qml/TextField.qml
        qml/TimelineTrack.qml
        qml/TimelineRuler.qml
        qml/TimelineInputHandler.qml
        qml/PlayheadOverlay.qml
        qml/TimelineView.qml
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
    $<INSTALL_INTERFACE:include/QtQuickComponents>
)

target_link_libraries(QtQuickComponents PUBLIC
    Qt6::Quick
    Qt6::QuickControls2
)

# ── 安装规则 ──────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS QtQuickComponents
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
    src/ComponentTheme.h
    src/TimelineSegment.h
    src/TimelineModel.h
    src/TimelineViewport.h
    src/RulerTick.h
    src/RulerModel.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QtQuickComponents
)
install(DIRECTORY qml/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/qml
)
install(DIRECTORY "${QTC_QML_OUTPUT_BASE}/QuickUI/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/QuickUI
)

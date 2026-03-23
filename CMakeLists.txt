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

# ── 库目标 ────────────────────────────────────────────────────
# 含 C++ 类型时用 SHARED：插件会被编译成 .dylib/.so，
# QML 引擎运行时按 qmldir 中的 plugin 行加载它。
# STATIC 库不会生成插件动态库，运行时会报 plugin not found。
qt_add_library(QtQuickComponents SHARED)

# OUTPUT_DIRECTORY 由调用方通过 QTC_QML_OUTPUT_BASE 传入，
# 确保 qmldir 和插件 .dylib 输出到同一目录，QML 引擎才能找到。
if(NOT DEFINED QTC_QML_OUTPUT_BASE)
    set(QTC_QML_OUTPUT_BASE "${CMAKE_BINARY_DIR}")
endif()

qt_add_qml_module(QtQuickComponents
    URI     "QuickUI.Components"
    VERSION 1.0
    OUTPUT_DIRECTORY "${QTC_QML_OUTPUT_BASE}/QuickUI/Components"
    SOURCES
        src/ComponentTheme.h
        src/ComponentTheme.cpp
    QML_FILES
        qml/IconButton.qml
        qml/ProgressSlider.qml
)

# 插件 .dylib 也必须输出到同一目录，QML 引擎才能按 qmldir 找到它
set_target_properties(QtQuickComponentsplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${QTC_QML_OUTPUT_BASE}/QuickUI/Components"
    RUNTIME_OUTPUT_DIRECTORY "${QTC_QML_OUTPUT_BASE}/QuickUI/Components"
)

target_include_directories(QtQuickComponents PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/QtQuickComponents>
)

target_link_libraries(QtQuickComponents PUBLIC
    Qt6::Quick
    Qt6::QuickControls2
)

# ── 安装规则（仅文件，不用 EXPORT 机制）──────────────────────
include(GNUInstallDirs)

install(TARGETS QtQuickComponents QtQuickComponentsplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES src/ComponentTheme.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QtQuickComponents
)
install(DIRECTORY qml/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/qml
)
install(DIRECTORY "${QTC_QML_OUTPUT_BASE}/QuickUI/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/QuickUI
)

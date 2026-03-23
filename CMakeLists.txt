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
qt_add_library(QtQuickComponents STATIC)

# OUTPUT_DIRECTORY 必须由调用方（VideoPlayer）通过
# QTC_QML_OUTPUT_BASE 传入，确保 qmldir 和插件输出到
# VideoPlayer 构建目录下，QML 引擎才能找到。
# 独立构建时回退到自己的 CMAKE_BINARY_DIR。
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

install(TARGETS QtQuickComponents
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
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

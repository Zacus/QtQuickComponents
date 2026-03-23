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

qt_add_qml_module(QtQuickComponents
    URI     "QuickUI.Components"
    VERSION 1.0
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/QuickUI/Components"
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

# ── 安装规则 ──────────────────────────────────────────────────
# 注意：qt_add_qml_module 会生成若干内部辅助 target（_resources_N、_init 等），
# 这些 target 无法通过 install(EXPORT) 安全导出（Qt 版本不同数量会变）。
# 因此安装只处理文件，不使用 EXPORT 机制。
# 使用者通过 FetchContent 引入时直接使用 build tree target，无需 install。
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

# qmldir 和生成的资源文件
install(DIRECTORY "${CMAKE_BINARY_DIR}/QuickUI/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/QtQuickComponents/QuickUI
)

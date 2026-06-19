cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED QTC_SOURCE_DIR)
    message(FATAL_ERROR "QTC_SOURCE_DIR is required")
endif()
if(NOT DEFINED QTC_QMLDIR_FILE)
    message(FATAL_ERROR "QTC_QMLDIR_FILE is required")
endif()
if(NOT EXISTS "${QTC_QMLDIR_FILE}")
    message(FATAL_ERROR "QML module qmldir not found: ${QTC_QMLDIR_FILE}")
endif()

set(_qtc_public_qml_files
    src/controls/ComboBox.qml
    src/controls/IconButton.qml
    src/controls/ProgressSlider.qml
    src/controls/Button.qml
    src/controls/Label.qml
    src/controls/TextField.qml
    src/timeline/TimelineView.qml
)

set(_qtc_internal_qml_files
    src/timeline/TimelineTrack.qml
    src/timeline/TimelineRuler.qml
    src/timeline/TimelineInputHandler.qml
    src/timeline/PlayheadOverlay.qml
)

file(READ "${QTC_QMLDIR_FILE}" _qtc_qmldir)

foreach(_qtc_public_qml_file IN LISTS _qtc_public_qml_files)
    get_filename_component(_qtc_type "${_qtc_public_qml_file}" NAME_WE)
    get_filename_component(_qtc_file "${_qtc_public_qml_file}" NAME)
    if(NOT _qtc_qmldir MATCHES "(^|\n)${_qtc_type} [0-9]+\\.[0-9]+ ${_qtc_file}(\n|$)")
        message(FATAL_ERROR
            "Public QML type '${_qtc_type}' must have a versioned qmldir entry")
    endif()
    if(_qtc_qmldir MATCHES "(^|\n)internal ${_qtc_type} ${_qtc_file}(\n|$)")
        message(FATAL_ERROR
            "Public QML type '${_qtc_type}' must not be marked internal")
    endif()
endforeach()

foreach(_qtc_internal_qml_file IN LISTS _qtc_internal_qml_files)
    get_filename_component(_qtc_type "${_qtc_internal_qml_file}" NAME_WE)
    get_filename_component(_qtc_file "${_qtc_internal_qml_file}" NAME)
    if(_qtc_qmldir MATCHES "(^|\n)${_qtc_type} [0-9]+\\.[0-9]+ ${_qtc_file}(\n|$)")
        message(FATAL_ERROR
            "Internal QML type '${_qtc_type}' must not have a public versioned qmldir entry")
    endif()
    if(NOT _qtc_qmldir MATCHES "(^|\n)internal ${_qtc_type} ${_qtc_file}(\n|$)")
        message(FATAL_ERROR
            "Internal QML type '${_qtc_type}' must have an internal qmldir entry")
    endif()
endforeach()

file(READ "${QTC_SOURCE_DIR}/CMakeLists.txt" _qtc_cmake)
string(REPLACE "\n" ";" _qtc_lines "${_qtc_cmake}")

set(_qtc_in_public_qml FALSE)
set(_qtc_public_qml_from_cmake)
foreach(_qtc_line IN LISTS _qtc_lines)
    string(STRIP "${_qtc_line}" _qtc_stripped)
    if(_qtc_stripped MATCHES "^set\\(QTC_PUBLIC_QML_FILES")
        set(_qtc_in_public_qml TRUE)
        continue()
    endif()

    if(_qtc_in_public_qml)
        if(_qtc_stripped STREQUAL ")")
            set(_qtc_in_public_qml FALSE)
            continue()
        endif()
        list(APPEND _qtc_public_qml_from_cmake "${_qtc_stripped}")
    endif()
endforeach()

foreach(_qtc_internal_qml_file IN LISTS _qtc_internal_qml_files)
    if(_qtc_internal_qml_file IN_LIST _qtc_public_qml_from_cmake)
        message(FATAL_ERROR
            "Internal QML file '${_qtc_internal_qml_file}' must not be listed in QTC_PUBLIC_QML_FILES")
    endif()
endforeach()

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED QTC_SOURCE_DIR)
    message(FATAL_ERROR "QTC_SOURCE_DIR is required")
endif()

file(READ "${QTC_SOURCE_DIR}/CMakeLists.txt" _qtc_cmake)
string(REPLACE "\n" ";" _qtc_lines "${_qtc_cmake}")

set(_qtc_in_public_headers FALSE)
set(_qtc_public_headers)
foreach(_qtc_line IN LISTS _qtc_lines)
    string(STRIP "${_qtc_line}" _qtc_stripped)
    if(_qtc_stripped MATCHES "^set\\(QTC_PUBLIC_HEADERS")
        set(_qtc_in_public_headers TRUE)
        continue()
    endif()

    if(_qtc_in_public_headers)
        if(_qtc_stripped STREQUAL ")")
            set(_qtc_in_public_headers FALSE)
            continue()
        endif()
        list(APPEND _qtc_public_headers "${_qtc_stripped}")
    endif()
endforeach()

set(_qtc_private_headers
    src/theme/ThemeFileWatcher.h
    src/theme/ThemeJsonLoader.h
    src/theme/ThemeTokens.h
    src/timeline/TimelineViewport.h
    src/timeline/RulerTick.h
    src/timeline/RulerModel.h
    src/timeline/TimelineTrackModel.h
)

foreach(_qtc_private_header IN LISTS _qtc_private_headers)
    if(_qtc_private_header IN_LIST _qtc_public_headers)
        message(FATAL_ERROR
            "Private header '${_qtc_private_header}' must not be listed in QTC_PUBLIC_HEADERS")
    endif()
endforeach()

foreach(_qtc_public_header IN LISTS _qtc_public_headers)
    if(NOT _qtc_public_header MATCHES "^include/QtQuickComponents/.+\\.h$")
        message(FATAL_ERROR
            "Public header '${_qtc_public_header}' must live under include/QtQuickComponents")
    endif()
endforeach()

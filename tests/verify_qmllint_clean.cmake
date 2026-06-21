cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED QTC_QMLLINT_EXECUTABLE)
    message(FATAL_ERROR "QTC_QMLLINT_EXECUTABLE is required")
endif()
if(NOT DEFINED QTC_QML_IMPORT_PATH)
    message(FATAL_ERROR "QTC_QML_IMPORT_PATH is required")
endif()

set(_qtc_modules
    QuickUI.Components
    QuickUI.Components.impl
)

foreach(_qtc_module IN LISTS _qtc_modules)
    execute_process(
        COMMAND
            "${QTC_QMLLINT_EXECUTABLE}"
            --max-warnings 0
            -I "${QTC_QML_IMPORT_PATH}"
            -M "${_qtc_module}"
        RESULT_VARIABLE _qtc_result
        OUTPUT_VARIABLE _qtc_stdout
        ERROR_VARIABLE _qtc_stderr
    )

    if(NOT _qtc_result EQUAL 0)
        message(FATAL_ERROR
            "qmllint failed for ${_qtc_module}\n"
            "stdout:\n${_qtc_stdout}\n"
            "stderr:\n${_qtc_stderr}"
        )
    endif()
endforeach()

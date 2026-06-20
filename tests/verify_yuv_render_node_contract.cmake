cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED QTC_SOURCE_DIR)
    message(FATAL_ERROR "QTC_SOURCE_DIR is required")
endif()

set(_qtc_render_node "${QTC_SOURCE_DIR}/src/monitor/Yuv420RenderNode.cpp")
if(NOT EXISTS "${_qtc_render_node}")
    message(FATAL_ERROR "Yuv420RenderNode.cpp not found: ${_qtc_render_node}")
endif()

file(READ "${_qtc_render_node}" _qtc_render_node_cpp)

if(NOT _qtc_render_node_cpp MATCHES
        "void[ \t\r\n]+Yuv420RenderNode::render\\([^)]*\\)[ \t\r\n]*\\{[ \t\r\n]*recordDrawCommands\\(commandBuffer\\(\\)\\);[ \t\r\n]*\\}")
    message(FATAL_ERROR
        "Yuv420RenderNode::render() must delegate to recordDrawCommands(commandBuffer()).")
endif()

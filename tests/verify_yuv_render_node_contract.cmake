cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED QTC_SOURCE_DIR)
    message(FATAL_ERROR "QTC_SOURCE_DIR is required")
endif()

set(_qtc_render_node "${QTC_SOURCE_DIR}/src/window/singleWnd/rendering/Yuv420RenderNode.cpp")
if(NOT EXISTS "${_qtc_render_node}")
    message(FATAL_ERROR "Yuv420RenderNode.cpp not found: ${_qtc_render_node}")
endif()

file(READ "${_qtc_render_node}" _qtc_render_node_cpp)

if(NOT _qtc_render_node_cpp MATCHES
        "void[ \t\r\n]+Yuv420RenderNode::render\\([^)]*\\)")
    message(FATAL_ERROR
        "Yuv420RenderNode::render() implementation was not found.")
endif()

if(NOT _qtc_render_node_cpp MATCHES
        "if[ \t\r\n]*\\([ \t\r\n]*!renderTarget\\(\\)[ \t\r\n]*\\|\\|[ \t\r\n]*!commandBuffer\\(\\)[ \t\r\n]*\\)")
    message(FATAL_ERROR
        "Yuv420RenderNode::render() must guard missing SceneGraph render resources.")
endif()

if(NOT _qtc_render_node_cpp MATCHES "projectionMatrix\\(\\)"
        OR NOT _qtc_render_node_cpp MATCHES "matrix\\(\\)"
        OR NOT _qtc_render_node_cpp MATCHES
            "renderFrame\\(renderTarget\\(\\),[ \t\r\n]*commandBuffer\\(\\),[ \t\r\n]*transform,[ \t\r\n]*float\\(inheritedOpacity\\(\\)\\)\\)")
    message(FATAL_ERROR
        "Yuv420RenderNode::render() must pass the SceneGraph transform into renderFrame().")
endif()

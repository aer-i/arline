cmake_minimum_required(VERSION 3.15)

project(arline)
option(ARLINE_BUILD_EXAMPLES "Build examples" ON)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/Vulkan-Headers)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/volk)

add_library(arline)
target_compile_features(arline PUBLIC cxx_std_20)
target_sources(arline PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/ArlineWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/ArlineVkContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/ArlinePipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/ArlineShader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/ArlineBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/ArlineImage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/ArlineImgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
)

target_include_directories(arline PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/VulkanMemoryAllocator/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/
)

target_link_libraries(arline PUBLIC glfw Vulkan::Headers volk)

if (ARLINE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
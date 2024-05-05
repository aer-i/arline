cmake_minimum_required(VERSION 3.15)

project(arline)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)

add_library(arline)
target_sources(arline PRIVATE
    arline/ArlineWindow.cpp
)

target_include_directories(arline PUBLIC
    arline/
)

target_link_libraries(arline PRIVATE
    glfw
)

include(examples/examples.cmake)
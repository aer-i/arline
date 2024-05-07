cmake_minimum_required(VERSION 3.15)

project(arline)

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
)

target_include_directories(arline PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/arline/
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/VulkanMemoryAllocator/include/
)

target_link_libraries(arline PUBLIC glfw Vulkan::Headers volk)

add_subdirectory(examples)
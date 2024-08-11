cmake_minimum_required(VERSION 3.26)

find_package(Vulkan REQUIRED)

project(arline C)
option(AR_BUILD_EXAMPLES "Build arline examples")
option(AR_ENABLE_INFO_CALLBACK "Enable info callback - should be disabled for release builds.")

add_library(arline)
target_sources(arline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/arline/arline.c)
target_include_directories(arline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/arline/)
target_link_libraries(arline PUBLIC Vulkan::Vulkan dwmapi)
set_target_properties(arline PROPERTIES LINK_FLAGS "/NODEFAULTLIB")
target_compile_options(arline PRIVATE /GS-)
target_compile_options(arline PRIVATE /GR-)
target_compile_options(arline PRIVATE /nologo)

if (AR_ENABLE_INFO_CALLBACK)
    target_compile_definitions(arline PRIVATE AR_ENABLE_INFO_CALLBACK)
endif()

if (AR_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
cmake_minimum_required(VERSION 3.26)

find_package(Vulkan REQUIRED)

project(arline C)
option(AR_BUILD_EXAMPLES "Build arline examples" ON)

add_library(arline)
target_sources(arline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/arline/arline.c)
target_include_directories(arline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/arline/)
target_link_libraries(arline PUBLIC dwmapi Vulkan::Vulkan)
set_target_properties(arline PROPERTIES LINK_FLAGS "/NODEFAULTLIB /NOLOGO")
target_link_options(arline PRIVATE /NODEFAULTLIB)
target_link_options(arline PRIVATE /NOLOGO)
target_compile_options(arline PRIVATE /W4)
target_compile_options(arline PRIVATE /GS-)
target_compile_options(arline PRIVATE /GR-)
target_compile_options(arline PRIVATE /nologo)

if (AR_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
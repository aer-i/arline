cmake_minimum_required(VERSION 3.26)

project(arline CXX)
option(AR_BUILD_EXAMPLES "Build arline examples")
option(AR_ENABLE_INFO_CALLBACK "Enable info callback - should be disabled for release builds.")

add_library(arline)
target_compile_features(arline PRIVATE cxx_std_23)
target_sources(arline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/arline/Arline.cxx)
target_include_directories(arline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/arline/)
target_compile_options(arline PRIVATE
$<$<CXX_COMPILER_ID:MSVC>:/W4>
$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wpedantic>
)

if (UNIX)
    target_link_libraries(arline PRIVATE X11)
endif()

if (AR_ENABLE_INFO_CALLBACK)
    target_compile_definitions(arline PRIVATE AR_ENABLE_INFO_CALLBACK)
endif()

if (AR_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
add_executable(2-indexBuffer example.cpp)
target_link_libraries(2-indexBuffer PUBLIC arline)

#compile shader to bin directory-------------------------------------

find_program(GLSL_VALIDATOR glslangValidator HINTS
    ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
    /usr/bin
    /usr/local/bin
    ${VULKAN_SDK_PATH}/Bin
    ${VULKAN_SDK_PATH}/Bin32
    $ENV{VULKAN_SDK}/Bin/
    $ENV{VULKAN_SDK}/Bin32/
)

if (NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "GLSL Validator not found")
endif ()

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "shaders/*.frag"
    "shaders/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "shaders/"
        COMMAND ${GLSL_VALIDATOR} --target-env vulkan1.3 -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders2
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(2-indexBuffer Shaders2)
add_executable(Triangle example.cxx)
target_link_libraries(Triangle PUBLIC arline)
target_compile_features(Triangle PUBLIC cxx_std_20)

if (MSVC)
    set_target_properties(Triangle PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_custom_command(TARGET Triangle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Triangle>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Triangle>/shaders"
)
add_executable(Cube example.cxx)
target_link_libraries(Cube PUBLIC arline)
target_compile_features(Cube PUBLIC cxx_std_20)


add_custom_command(TARGET Cube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Cube>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Cube>/shaders"
)
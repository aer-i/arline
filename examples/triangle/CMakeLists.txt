add_executable(Triangle example.c)
target_link_libraries(Triangle PUBLIC arline)

set_target_properties(Triangle PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:main /NODEFAULTLIB /NOLOGO")
set_target_properties(Triangle PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:main /NODEFAULTLIB /NOLOGO")
target_link_options(arline PRIVATE /NODEFAULTLIB)
target_link_options(arline PRIVATE /NOLOGO)
target_link_options(arline PRIVATE /entry:main)
target_link_options(arline PRIVATE /SUBSYSTEM:WINDOWS)
target_compile_options(Triangle PRIVATE /nologo)
target_compile_options(Triangle PRIVATE /EHs-c-)
target_compile_options(Triangle PRIVATE /D_HAS_EXCEPTIONS=0)
target_compile_options(Triangle PRIVATE /GR-)
target_compile_options(Triangle PRIVATE /Gm-)
target_compile_options(Triangle PRIVATE /Gy)
target_compile_options(Triangle PRIVATE /GS-)
target_compile_options(Triangle PRIVATE /MP)
target_compile_options(Triangle PRIVATE /fp:fast)
target_compile_options(Triangle PRIVATE /fp:except-)
target_compile_options(Triangle PRIVATE /Zc:inline)

add_custom_command(TARGET Triangle PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Triangle>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Triangle>/shaders"
)
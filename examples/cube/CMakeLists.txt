add_executable(Cube example.cxx)
target_link_libraries(Cube PUBLIC arline)
target_compile_features(Cube PRIVATE cxx_std_20)

if(MSVC)
    set_target_properties(Cube PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /NOLOGO")
    set_target_properties(Cube PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /NOLOGO")
    target_compile_options(Cube PRIVATE /nologo)
    target_compile_options(Cube PRIVATE /EHs-c-)
    target_compile_options(Cube PRIVATE /D_HAS_EXCEPTIONS=0)
    target_compile_options(Cube PRIVATE /GR-)
    target_compile_options(Cube PRIVATE /Gm-)
    target_compile_options(Cube PRIVATE /Gy)
    target_compile_options(Cube PRIVATE /GS-)
    target_compile_options(Cube PRIVATE /MP)
    target_compile_options(Cube PRIVATE /fp:fast)
    target_compile_options(Cube PRIVATE /fp:except-)
    target_compile_options(Cube PRIVATE /Zc:inline)
endif()

add_custom_command(TARGET Cube PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Cube>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Cube>/shaders"
)
add_executable(Cube example.cxx)
target_link_libraries(Cube PUBLIC arline)
target_compile_features(Cube PUBLIC cxx_std_20)

if (MSVC)
    set_target_properties(Cube PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
add_executable(Triangle example.cxx)
target_link_libraries(Triangle PUBLIC arline)
target_compile_features(Triangle PUBLIC cxx_std_20)

if (MSVC)
    set_target_properties(Triangle PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
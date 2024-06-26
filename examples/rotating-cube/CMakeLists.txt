add_executable(Cube example.cxx)
target_link_libraries(Cube PUBLIC arline)
target_compile_features(Cube PUBLIC cxx_std_20)
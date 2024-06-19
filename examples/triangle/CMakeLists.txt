set(ARLINE_INFO_CALLBACK ON CACHE BOOL "" FORCE)
add_executable(Triangle example.cpp)
target_link_libraries(Triangle PUBLIC arline)
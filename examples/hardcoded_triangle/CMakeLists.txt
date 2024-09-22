add_executable(
    hardcoded_triangle
    src/main.cpp
)
target_link_libraries(
    hardcoded_triangle PRIVATE engine
)
compile_shaders(hardcoded_triangle)
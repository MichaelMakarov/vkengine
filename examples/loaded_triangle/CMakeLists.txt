add_executable(
    loaded_triangle
    src/main.cpp
)
target_link_libraries(
    loaded_triangle PRIVATE engine glm::glm
)
compile_shaders(loaded_triangle)
add_executable(
    triangle_color
    src/main.cpp
)
target_link_libraries(
    triangle_color PRIVATE engine
)
compile_shaders(triangle_color)
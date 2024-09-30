add_executable(
    triangle_switch_color
    src/main.cpp
)
target_link_libraries(
    triangle_switch_color PRIVATE engine
)
compile_shaders(triangle_switch_color)
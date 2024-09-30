add_executable(
    triangle_from_memory
    src/main.cpp
)
target_link_libraries(
    triangle_from_memory PRIVATE engine
)
compile_shaders(triangle_from_memory)
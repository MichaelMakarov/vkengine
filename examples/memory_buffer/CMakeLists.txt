add_executable(
    memory_buffer
    src/main.cpp
)
target_link_libraries(
    memory_buffer PRIVATE engine
)
compile_shaders(memory_buffer)
add_executable(
    memory_buffer
    src/main.cpp
    src/matrix.cpp
    src/plain_mesh.cpp
)
target_link_libraries(
    memory_buffer PRIVATE engine
)
compile_shaders(memory_buffer)
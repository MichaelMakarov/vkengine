add_executable(
    memory_buffer
    src/main.cpp
    src/matrix_descriptor.cpp
    src/pipeline_provider.cpp
    src/plain_mesh.cpp
    src/texture_descriptor.cpp
)
target_link_libraries(
    memory_buffer PRIVATE engine
)
compile_shaders(memory_buffer)
copy_images()
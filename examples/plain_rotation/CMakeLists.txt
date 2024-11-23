add_executable(
    plain_rotation_example
    src/main.cpp
    src/matrix_descriptor.cpp
    src/pipeline_provider.cpp
    src/plain_mesh.cpp
    src/texture_descriptor.cpp
)
target_link_libraries(
    plain_rotation_example PRIVATE 
    engine 
    glm::glm
)
compile_shaders(plain_rotation_example)
copy_images()
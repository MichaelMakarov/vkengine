add_executable(
    triangle_color_example
    src/main.cpp
    src/pipeline_provider.cpp
    src/triangle_pipeline.cpp
)
target_link_libraries(
    triangle_color_example PRIVATE engine
)
compile_shaders(triangle_color_example)
add_library(
    engine STATIC
    src/engine.cpp
    src/renderer.cpp
    src/core.cpp
    src/pipeline.cpp
    src/copy.cpp
    src/sync.cpp
    src/command.cpp
    src/delete.cpp
)
target_include_directories(
    engine PUBLIC include
)
target_link_libraries(
    engine PUBLIC 
    utility
    Vulkan::Vulkan
    glm::glm
    glfw
)
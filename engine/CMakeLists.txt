add_library(
    engine STATIC
    src/error.cpp
    src/engine.cpp
    src/renderer.cpp
    src/core.cpp
    src/pipeline.cpp
    src/swapchain.cpp
    src/copy.cpp
    src/sync.cpp
    src/command.cpp
    src/shaders.cpp
    src/delete.cpp
)
target_include_directories(
    engine PUBLIC include
)
target_link_libraries(
    engine
    PRIVATE
    Vulkan::Vulkan
    glfw
    PUBLIC
    glm::glm
)
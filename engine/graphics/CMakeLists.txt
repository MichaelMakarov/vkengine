add_library(
    engine_graphics STATIC
    buffer_manager.cpp
    device_context.cpp 
    graphics_manager.cpp 
    graphics_renderer.cpp 
    image_context.cpp 
    image_renderer.cpp 
    instance_context.cpp 
    memory_buffer.cpp
    physical_device.cpp 
    pipeline_builder.cpp
    swapchain_context.cpp 
    swapchain_presenter.cpp 
    window_context.cpp
)
target_include_directories(
    engine_graphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    engine_graphics PUBLIC 
    engine_utility
    Vulkan::Vulkan
    glfw
)
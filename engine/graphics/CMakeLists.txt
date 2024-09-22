add_library(
    engine_graphics STATIC
    device_context.cpp 
    graphics_renderer.cpp 
    image_context.cpp 
    image_renderer.cpp 
    instance_context.cpp 
    physical_device.cpp 
    swapchain_context.cpp 
    swapchain_presenter.cpp 
    graphics_manager.cpp 
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
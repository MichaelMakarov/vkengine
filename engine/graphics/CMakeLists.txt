add_library(
    engine_graphics STATIC
    buffer_copy_command.cpp
    commander.cpp
    depth_texture.cpp
    descriptor_set.cpp
    device_context.cpp 
    graphics_manager.cpp 
    graphics_renderer.cpp 
    image_copy_command.cpp
    image_renderer.cpp 
    image_transition_command.cpp
    instance_context.cpp 
    memory_barrier.cpp
    memory_barrier_command.cpp
    memory_list_allocator.cpp
    memory_block.cpp
    memory_buffer.cpp
    pipeline_builder.cpp
    shader_context.cpp
    swapchain_context.cpp 
    swapchain_presenter.cpp 
    image_texture.cpp
    commander.cpp
    window_context.cpp
)
target_include_directories(
    engine_graphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    engine_graphics 
    PUBLIC 
    engine_utility
    Vulkan::Vulkan
    glfw
    PRIVATE 
    engine_image
)
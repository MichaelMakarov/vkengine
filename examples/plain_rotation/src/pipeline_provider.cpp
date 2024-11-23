#include "pipeline_provider.hpp"

#include "graphics/graphics_manager.hpp"

#include <array>

PipelineProvider::PipelineProvider(shared_ptr_of<VkDevice> device,
                                   VkPhysicalDevice phys_device,
                                   uint32_t transfer_qfm,
                                   uint32_t graphics_qfm,
                                   std::shared_ptr<AllocatorInterface> allocator)
    : allocator_{allocator}
    , transfer_{device, transfer_qfm, 0}
    , barrier_{device, graphics_qfm, 0}
    , mesh_{device, allocator_, transfer_}
    , texture_{std::make_shared<TextureDescriptor>(device, allocator_, transfer_, barrier_)}
    , matrix_{std::make_shared<MatrixDescriptor>(device)}
    , descriptor_set_{device, {matrix_, texture_}}
    , builder_{device}
    , vertex_shader_{device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT}
    , fragment_shader_{device, "shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
    , pipeline_layout_{
          GraphicsManager::make_pipeline_layout(device, std::array<VkDescriptorSetLayout, 1>{descriptor_set_.get_layout()}, {})} {
    builder_.set_shader_stages({vertex_shader_.get_shader_stage(), fragment_shader_.get_shader_stage()});
    builder_.set_pipeline_layout(pipeline_layout_);
    VertexInputStateProvider vertex_input_state;
    vertex_input_state.set_vertex_bindings({mesh_.get_vertex_binding_description()});
    vertex_input_state.set_vertex_attributes(mesh_.get_vertex_attribute_descriptions());
    builder_.set_vertex_input_state(std::move(vertex_input_state));
    builder_.set_depth_stencil_state(VkPipelineDepthStencilStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0,
        .maxDepthBounds = 1,

    });
}

void PipelineProvider::update_command_buffer(VkCommandBuffer command_buffer, size_t image_index) {
    VkDescriptorSet descriptor_set = descriptor_set_.get_descriptor_set(image_index);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_.get(), 0, 1, &descriptor_set, 0, nullptr);
    mesh_.draw(command_buffer);
}

void PipelineProvider::setup_pipeline(GraphicsRenderer::Context const &info) {
    matrix_->setup_buffers(info.images_count, info.surface_extent, allocator_);
    descriptor_set_.set_swapchain_images_count(info.images_count);
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(info.surface_extent.width),
        .height = static_cast<float>(info.surface_extent.height),
        .minDepth = 0,
        .maxDepth = 1,
    };
    VkRect2D scissor{.offset = {.x = 0, .y = 0}, .extent = info.surface_extent};
    ViewportStateProvider viewport_state;
    viewport_state.set_viewports({viewport});
    viewport_state.set_scissors({scissor});
    builder_.set_viewport_state(std::move(viewport_state));
    builder_.set_render_pass(info.render_pass);
    pipeline_ = builder_.make_pipeline();
}

void PipelineProvider::update_image(size_t image_index) {
    matrix_->update_content(image_index);
}

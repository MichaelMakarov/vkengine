#include "triangle_pipeline.hpp"

#include "graphics/graphics_manager.hpp"

TrianglePipeline::TrianglePipeline(shared_ptr_of<VkDevice> device, std::string_view vertex_shader, std::string_view fragment_shader)
    : vertex_shader_{device, vertex_shader, VK_SHADER_STAGE_VERTEX_BIT}
    , fragment_shader_{device, fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT}
    , pipeline_builder_{device} {
    pipeline_builder_.set_shader_stages({vertex_shader_.get_shader_stage(), fragment_shader_.get_shader_stage()});
    pipeline_builder_.set_pipeline_layout(GraphicsManager::make_pipeline_layout(device, {}, {}));
}

void TrianglePipeline::update_pipeline(GraphicsRenderer::Context const &context) {
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(context.surface_extent.width),
        .height = static_cast<float>(context.surface_extent.height),
        .minDepth = 0,
        .maxDepth = 1,
    };
    VkRect2D scissor{.offset = {.x = 0, .y = 0}, .extent = context.surface_extent};
    ViewportStateProvider viewport_provider;
    viewport_provider.set_viewports({viewport});
    viewport_provider.set_scissors({scissor});
    pipeline_builder_.set_viewport_state(std::move(viewport_provider));
    pipeline_builder_.set_render_pass(context.render_pass);
    pipeline_ = pipeline_builder_.make_pipeline();
}

void TrianglePipeline::draw(VkCommandBuffer command_buffer) {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());
    vkCmdDraw(command_buffer, 3, 1, 0, 0);
}

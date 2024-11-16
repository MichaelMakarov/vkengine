#pragma once

#include "graphics/graphics_types.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/shader_context.hpp"
#include "graphics/graphics_renderer.hpp"

#include <vector>

class TrianglePipeline {
    unique_ptr_of<VkPipeline> pipeline_;
    ShaderContext vertex_shader_;
    ShaderContext fragment_shader_;
    PipelineBuilder pipeline_builder_;

  public:
    TrianglePipeline(shared_ptr_of<VkDevice> device, std::string_view vertex_shader, std::string_view fragment_shader);

    void update_pipeline(GraphicsRenderer::Context const &context);

    void draw(VkCommandBuffer command_buffer);
};
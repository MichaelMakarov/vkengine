#pragma once

#include "triangle_pipeline.hpp"

class PipelineProvider {
    TrianglePipeline colorful_triangle_;
    TrianglePipeline monochrome_triangle_;
    TrianglePipeline *current_triangle_ = nullptr;

  public:
    explicit PipelineProvider(shared_ptr_of<VkDevice> device);

    void update_command_buffer(VkCommandBuffer command_buffer, size_t index);

    void setup_pipeline(GraphicsRenderer::Context const &info);

    void change_pipeline();
};
#include "pipeline_provider.hpp"

PipelineProvider::PipelineProvider(shared_ptr_of<VkDevice> device)
    : colorful_triangle_{device, "colorful_triangle.vert.spv", "colorful_triangle.frag.spv"}
    , monochrome_triangle_{device, "monochrome_triangle.vert.spv", "monochrome_triangle.frag.spv"}
    , current_triangle_{&monochrome_triangle_} {
}

void PipelineProvider::update_command_buffer(VkCommandBuffer command_buffer, size_t index) {
    current_triangle_->draw(command_buffer);
}

void PipelineProvider::setup_pipeline(GraphicsRenderer::Context const &info) {
    colorful_triangle_.update_pipeline(info);
    monochrome_triangle_.update_pipeline(info);
}

void PipelineProvider::change_pipeline() {
    if (current_triangle_ == &monochrome_triangle_) {
        current_triangle_ = &colorful_triangle_;
    } else {
        current_triangle_ = &monochrome_triangle_;
    }
}

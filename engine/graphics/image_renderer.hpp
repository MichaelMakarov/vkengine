#pragma once

#include "graphics_types.hpp"

#include "image_context.hpp"

class ImageRenderer {
    ImageContext const *image_ptr_{nullptr};
    VkRenderPass render_pass_;
    VkRect2D rect_;

  public:
    ImageRenderer() = default;

    ImageRenderer(ImageContext const *image_ctx, VkRenderPass render_pass, VkExtent2D extent);

    VkCommandBuffer begin_render_pass();

    void end_render_pass();
};
#pragma once

#include "graphics_types.hpp"

#include <span>

class ImageRenderer {
    VkCommandBuffer command_buffer_;
    VkFramebuffer framebuffer_;
    VkRenderPass render_pass_;
    VkRect2D rect_;
    std::span<VkClearValue const> clear_values_;

  public:
    ImageRenderer() = default;

    ImageRenderer(VkCommandBuffer command_buffer,
                  VkFramebuffer framebuffer,
                  VkRenderPass render_pass,
                  VkExtent2D extent,
                  std::span<VkClearValue const> clear_values)
        : command_buffer_{command_buffer}
        , framebuffer_{framebuffer}
        , render_pass_{render_pass}
        , rect_{.offset = {0, 0}, .extent = extent}
        , clear_values_{clear_values} {
    }

    VkCommandBuffer begin_render_pass();

    void end_render_pass();
};
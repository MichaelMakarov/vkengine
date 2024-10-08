#pragma once

#include "image_context.hpp"
#include "image_renderer.hpp"

#include <vector>

class SwapchainContext {
  public:
    struct config {
        VkSurfaceKHR surface;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkSurfaceTransformFlagBitsKHR pre_transform;
        VkCompositeAlphaFlagBitsKHR composite_alpha;
        VkImageUsageFlags image_usage;
        uint32_t images_count;
        uint32_t graphics_qfm;
        uint32_t present_qfm;
    };

  private:
    shared_ptr_of<VkDevice> device_;
    shared_ptr_of<VkCommandPool> command_pool_;
    std::vector<uint32_t> qfm_indices_;
    VkSwapchainCreateInfoKHR swapchain_info_;
    unique_ptr_of<VkSwapchainKHR> swapchain_;
    unique_ptr_of<VkRenderPass> render_pass_;
    std::vector<ImageContext> image_contexts_;
    size_t image_index_ = 0;

  public:
    SwapchainContext(shared_ptr_of<VkDevice> device, config const &info);

    ~SwapchainContext();

    ImageContext const &get_image();

    VkSwapchainKHR get_swapchain() const {
        return swapchain_.get();
    }

    VkRenderPass get_render_pass() const {
        return render_pass_.get();
    }

    void update_extent(VkExtent2D extent);

    std::vector<ImageRenderer> get_image_renderers() const;

  private:
    static std::vector<uint32_t> make_queue_family_indices(config const &info);
};
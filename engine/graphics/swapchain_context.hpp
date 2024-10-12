#pragma once

#include "image_context.hpp"
#include "image_renderer.hpp"

#include <vector>

class SwapchainContext {
  public:
    struct Config {
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
    class IndexSequence {
        size_t size_;
        size_t index_;

      public:
        IndexSequence(size_t size = 0)
            : size_{size}
            , index_{0} {
        }

        size_t get_index() {
            size_t index = index_++;
            index_ %= size_;
            return index;
        }
    };

    shared_ptr_of<VkDevice> device_;
    shared_ptr_of<VkCommandPool> command_pool_;
    std::vector<uint32_t> qfm_indices_;
    VkSwapchainCreateInfoKHR swapchain_info_;
    unique_ptr_of<VkSwapchainKHR> swapchain_;
    unique_ptr_of<VkRenderPass> render_pass_;
    std::vector<ImageContext> image_contexts_;
    IndexSequence index_sequence_;

  public:
    SwapchainContext(shared_ptr_of<VkDevice> device, Config const &info);

    ~SwapchainContext();

    ImageContext const &get_image();

    VkSwapchainKHR get_swapchain() const {
        return swapchain_.get();
    }

    VkRenderPass get_render_pass() const {
        return render_pass_.get();
    }

    uint32_t get_images_count() const {
        return static_cast<uint32_t>(image_contexts_.size());
    }

    void update_extent(VkExtent2D extent);

    std::vector<ImageRenderer> get_image_renderers() const;

  private:
    static std::vector<uint32_t> make_queue_family_indices(Config const &info);
};
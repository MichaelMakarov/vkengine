#pragma once

#include "allocator_interface.hpp"
#include "depth_texture.hpp"
#include "image_context.hpp"
#include "image_renderer.hpp"

#include <optional>
#include <vector>

class SwapchainContext {
    class IndexSequence {
        size_t size_;
        size_t index_;

      public:
        IndexSequence(size_t size = 0);

        size_t get_index();
    };

    class QfmContainer {
        uint32_t array_[2];
        uint32_t size_;

      public:
        QfmContainer(uint32_t graphics_qfm, uint32_t present_qfm);

        uint32_t const *data() const;

        uint32_t size() const;
    };

    shared_ptr_of<VkDevice> device_;
    shared_ptr_of<VkCommandPool> command_pool_;
    unique_ptr_of<VkSwapchainKHR> swapchain_;
    unique_ptr_of<VkRenderPass> render_pass_;
    QfmContainer qfm_indices_;
    std::unique_ptr<DepthTexture> depth_texture_;
    IndexSequence index_sequence_;
    VkSwapchainCreateInfoKHR swapchain_info_;
    VkAttachmentDescription color_attachment_;
    std::vector<VkAttachmentDescription> attachment_descriptions_;
    std::vector<VkAttachmentReference> color_attachments_;
    VkAttachmentReference depth_attachement_;
    std::vector<VkClearValue> clear_values_;
    VkSubpassDescription subpass_;
    VkSubpassDependency dependency_;
    VkRenderPassCreateInfo render_pass_info_;
    std::vector<ImageContext> image_contexts_;

  public:
    struct Info {
        struct SwapchainInfo {
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
        struct DepthInfo {
            VkImageTiling depth_tiling;
            VkFormat depth_format;
        };
        SwapchainInfo swapchain_info;
        std::optional<DepthInfo> depth_info;
    };

    SwapchainContext(shared_ptr_of<VkDevice> device, std::shared_ptr<AllocatorInterface> allocator, Info const &info);

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

    void update_extent(VkExtent2D extent, VkQueue graphics_queue);

    std::vector<ImageRenderer> get_image_renderers() const;
};
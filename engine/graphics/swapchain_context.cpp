#include "swapchain_context.hpp"

#include "commander.hpp"
#include "graphics_error.hpp"
#include "graphics_manager.hpp"

#include <array>

namespace {

    std::vector<VkImage> get_swapchain_images(VkDevice device, VkSwapchainKHR swapchain) {
        std::uint32_t images_count;
        vkGetSwapchainImagesKHR(device, swapchain, &images_count, nullptr);
        std::vector<VkImage> images(images_count);
        vkGetSwapchainImagesKHR(device, swapchain, &images_count, images.data());
        return images;
    }

} // namespace

SwapchainContext::IndexSequence::IndexSequence(size_t size)
    : size_{size}
    , index_{0} {
}

size_t SwapchainContext::IndexSequence::get_index() {
    size_t index = index_;
    index_ = (index_ + 1) % size_;
    return index;
}

SwapchainContext::QfmContainer::QfmContainer(uint32_t graphics_qfm, uint32_t present_qfm)
    : size_{1} {
    array_[0] = graphics_qfm;
    if (graphics_qfm != present_qfm) {
        array_[1] = present_qfm;
        size_ = 2;
    }
}

uint32_t const *SwapchainContext::QfmContainer::data() const {
    return array_;
}

uint32_t SwapchainContext::QfmContainer::size() const {
    return size_;
}

SwapchainContext::SwapchainContext(shared_ptr_of<VkDevice> device, std::shared_ptr<AllocatorInterface> allocator, Info const &info)
    : device_{device}
    , command_pool_{GraphicsManager::make_command_pool(device,
                                                       VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                       info.swapchain_info.graphics_qfm)}
    , qfm_indices_{info.swapchain_info.graphics_qfm, info.swapchain_info.present_qfm} {
    swapchain_info_ = VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = info.swapchain_info.surface,
        .minImageCount = info.swapchain_info.images_count,
        .imageFormat = info.swapchain_info.surface_format.format,
        .imageColorSpace = info.swapchain_info.surface_format.colorSpace,
        .imageExtent = VkExtent2D{},
        .imageArrayLayers = 1,
        .imageUsage = info.swapchain_info.image_usage,
        .imageSharingMode = qfm_indices_.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = qfm_indices_.size(),
        .pQueueFamilyIndices = qfm_indices_.data(),
        .preTransform = info.swapchain_info.pre_transform,
        .compositeAlpha = info.swapchain_info.composite_alpha,
        .presentMode = info.swapchain_info.present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr,
    };

    attachment_descriptions_.reserve(2);
    color_attachments_.reserve(1);
    clear_values_.reserve(2);
    attachment_descriptions_.push_back(VkAttachmentDescription{
        .format = info.swapchain_info.surface_format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    });
    color_attachments_.push_back(VkAttachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    });
    clear_values_.push_back(VkClearValue{.color = VkClearColorValue{0, 0, 0, 1}});
    depth_attachement_ = VkAttachmentReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    if (info.depth_info.has_value()) {
        depth_texture_ = std::make_unique<DepthTexture>(device, allocator, info.depth_info->depth_tiling, info.depth_info->depth_format);
        attachment_descriptions_.push_back(VkAttachmentDescription{
            .format = info.depth_info->depth_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        });
        clear_values_.push_back(VkClearValue{.depthStencil = VkClearDepthStencilValue{1, 0}});
    }

    subpass_ = VkSubpassDescription{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(color_attachments_.size()),
        .pColorAttachments = color_attachments_.data(),
        .pDepthStencilAttachment = info.depth_info.has_value() ? &depth_attachement_ : nullptr,
    };
    dependency_ = VkSubpassDependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    render_pass_info_ = VkRenderPassCreateInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachment_descriptions_.size()),
        .pAttachments = attachment_descriptions_.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass_,
        .dependencyCount = 1,
        .pDependencies = &dependency_,
    };
}

SwapchainContext::~SwapchainContext() = default;

ImageContext const &SwapchainContext::get_image() {
    return image_contexts_[index_sequence_.get_index()];
}

void SwapchainContext::update_extent(VkExtent2D extent, VkQueue graphics_queue) {
    std::vector<VkImageView> image_views(1);
    if (depth_texture_) {
        Commander transition_commander{device_, command_pool_, graphics_queue};
        depth_texture_->update_extent(extent, transition_commander);
        image_views.resize(2);
        image_views[1] = depth_texture_->get_image_view();
    }

    swapchain_info_.imageExtent = extent;
    swapchain_info_.oldSwapchain = swapchain_.get();
    swapchain_ = GraphicsManager::make_swapchain(device_, swapchain_info_);
    render_pass_ = GraphicsManager::make_render_pass(device_, render_pass_info_);

    auto images = get_swapchain_images(device_.get(), swapchain_.get());
    size_t prev_size = image_contexts_.size();
    image_contexts_.resize(images.size());
    // fill new images
    for (size_t i = 0; i < image_contexts_.size(); ++i) {
        ImageContext &image_context = image_contexts_[i];
        image_context.set_image_view(
            GraphicsManager::make_image_view(device_, images[i], swapchain_info_.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT));
        image_views[0] = image_context.get_image_view();
        image_context.set_framebuffer(
            GraphicsManager::make_framebuffer(device_, image_views, render_pass_.get(), swapchain_info_.imageFormat, extent));
    }
    // fill only for new count of images
    for (size_t i = prev_size; i < image_contexts_.size(); ++i) {
        ImageContext &image_context = image_contexts_[i];
        image_context.set_command_buffer(GraphicsManager::make_command_buffer(device_, command_pool_));
        image_context.set_submit_semaphore(GraphicsManager::make_semaphore(device_));
        image_context.set_present_semaphore(GraphicsManager::make_semaphore(device_));
    }
    index_sequence_ = IndexSequence(image_contexts_.size());
}

std::vector<ImageRenderer> SwapchainContext::get_image_renderers() const {
    std::vector<ImageRenderer> image_renderers;
    image_renderers.reserve(image_contexts_.size());
    for (auto const &image : image_contexts_) {
        image_renderers.emplace_back(image.get_command_buffer(),
                                     image.get_framebuffer(),
                                     render_pass_.get(),
                                     swapchain_info_.imageExtent,
                                     std::span(clear_values_));
    }
    return image_renderers;
}

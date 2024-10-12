#include "swapchain_context.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

namespace {

    std::vector<VkImage> get_swapchain_images(VkDevice device, VkSwapchainKHR swapchain) {
        std::uint32_t images_count;
        vkGetSwapchainImagesKHR(device, swapchain, &images_count, nullptr);
        std::vector<VkImage> images(images_count);
        vkGetSwapchainImagesKHR(device, swapchain, &images_count, images.data());
        return images;
    }

} // namespace

SwapchainContext::SwapchainContext(shared_ptr_of<VkDevice> device, Config const &info)
    : device_{device}
    , command_pool_{GraphicsManager::make_command_pool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, info.graphics_qfm)}
    , qfm_indices_{make_queue_family_indices(info)}
    , swapchain_info_{
          .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
          .pNext = nullptr,
          .flags = {},
          .surface = info.surface,
          .minImageCount = info.images_count,
          .imageFormat = info.surface_format.format,
          .imageColorSpace = info.surface_format.colorSpace,
          .imageExtent = VkExtent2D{},
          .imageArrayLayers = 1,
          .imageUsage = info.image_usage,
          .imageSharingMode = qfm_indices_.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
          .queueFamilyIndexCount = static_cast<uint32_t>(qfm_indices_.size()),
          .pQueueFamilyIndices = qfm_indices_.data(),
          .preTransform = info.pre_transform,
          .compositeAlpha = info.composite_alpha,
          .presentMode = info.present_mode,
          .clipped = VK_TRUE,
          .oldSwapchain = nullptr,
      } {
}

SwapchainContext::~SwapchainContext() = default;

ImageContext const &SwapchainContext::get_image() {
    return image_contexts_[index_sequence_.get_index()];
}

void SwapchainContext::update_extent(VkExtent2D extent) {
    swapchain_info_.imageExtent = extent;
    swapchain_info_.oldSwapchain = swapchain_.get();
    swapchain_ = GraphicsManager::make_swapchain(device_, swapchain_info_);
    render_pass_ = GraphicsManager::make_render_pass(device_, swapchain_info_.imageFormat);

    auto images = get_swapchain_images(device_.get(), swapchain_.get());
    size_t prev_size = image_contexts_.size();
    image_contexts_.resize(images.size());
    // fill new images
    for (size_t i = 0; i < image_contexts_.size(); ++i) {
        ImageContext &image_context = image_contexts_[i];
        image_context.set_image_view(GraphicsManager::make_image_view(device_, images[i], swapchain_info_.imageFormat));
        image_context.set_framebuffer(GraphicsManager::make_framebuffer(device_,
                                                                        image_context.get_image_view(),
                                                                        render_pass_.get(),
                                                                        swapchain_info_.imageFormat,
                                                                        extent));
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
        image_renderers.emplace_back(image.get_command_buffer(), image.get_framebuffer(), render_pass_.get(), swapchain_info_.imageExtent);
    }
    return image_renderers;
}

std::vector<uint32_t> SwapchainContext::make_queue_family_indices(Config const &info) {
    std::vector<uint32_t> qfm_indices;
    qfm_indices.reserve(2);
    qfm_indices.push_back(info.graphics_qfm);
    if (info.graphics_qfm != info.present_qfm) {
        qfm_indices.push_back(info.present_qfm);
    }
    return qfm_indices;
}

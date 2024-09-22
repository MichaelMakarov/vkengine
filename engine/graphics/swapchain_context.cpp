#include "swapchain_context.hpp"

#include "graphics_manager.hpp"
#include "graphics_error.hpp"

namespace {

    std::vector<VkImage> get_swapchain_images(VkDevice device, VkSwapchainKHR swapchain) {
        std::uint32_t images_count;
        vkGetSwapchainImagesKHR(device, swapchain, &images_count, nullptr);
        std::vector<VkImage> images(images_count);
        vkGetSwapchainImagesKHR(device, swapchain, &images_count, images.data());
        return images;
    }

} // namespace

swapchain_context::swapchain_context(shared_ptr_of<VkDevice> device, context_info const &info)
    : device_{device}
    , command_pool_{graphics_manager::make_command_pool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, info.graphics_qfm)}
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

swapchain_context::~swapchain_context() = default;

image_context const &swapchain_context::get_image() {
    image_context const &image_ctx = image_contexts_[image_index_];
    image_index_ = (image_index_ + 1) % image_contexts_.size();
    return image_ctx;
}

void swapchain_context::update_extent(VkExtent2D extent) {
    swapchain_info_.imageExtent = extent;
    swapchain_info_.oldSwapchain = swapchain_.get();
    swapchain_ = graphics_manager::make_swapchain(device_, swapchain_info_);
    render_pass_ = graphics_manager::make_render_pass(device_, swapchain_info_.imageFormat);

    auto images = get_swapchain_images(device_.get(), swapchain_.get());
    image_contexts_.resize(images.size());
    image_context::context_info info;
    info.extent = extent;
    info.format = swapchain_info_.imageFormat;
    info.render_pass = render_pass_.get();
    for (std::size_t i{}; i < images.size(); ++i) {
        info.image = images[i];
        image_contexts_[i] = image_context(device_, info, command_pool_);
    }
    image_index_ = 0;
}

std::vector<image_renderer> swapchain_context::get_image_renderers() const {
    std::vector<image_renderer> image_renderers;
    image_renderers.reserve(image_contexts_.size());
    for (auto const &image : image_contexts_) {
        image_renderers.emplace_back(&image, render_pass_.get(), swapchain_info_.imageExtent);
    }
    return image_renderers;
}

std::vector<uint32_t> swapchain_context::make_queue_family_indices(context_info const &info) {
    std::vector<uint32_t> qfm_indices;
    qfm_indices.reserve(2);
    qfm_indices.push_back(info.graphics_qfm);
    if (info.graphics_qfm != info.present_qfm) {
        qfm_indices.push_back(info.present_qfm);
    }
    return qfm_indices;
}

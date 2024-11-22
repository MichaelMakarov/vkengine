#include "graphics_renderer.hpp"

#include "graphics_error.hpp"
#include "memory_list_allocator.hpp"
#include "window_context.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>

inline bool operator==(VkSurfaceFormatKHR left, VkSurfaceFormatKHR right) {
    return left.colorSpace == right.colorSpace && left.format == right.format;
}

namespace {

    template <typename T>
    bool contains(std::vector<T> const &array, T const &elem) {
        return array.end() != std::find(array.begin(), array.end(), elem);
    }

    uint32_t minmax(uint32_t min, uint32_t value, uint32_t max) {
        return std::max(min, std::min(value, max));
    }

    char const *tiling_to_str(VkImageTiling tiling) {
        switch (tiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return "optimal tiling";
        case VK_IMAGE_TILING_LINEAR:
            return "linear tiling";
        default:
            return "unknown tiling";
        }
    }

} // namespace

GraphicsRenderer::GraphicsRenderer(WindowConfig const &info)
    : GraphicsRenderer(info, Config{}) {
}

GraphicsRenderer::GraphicsRenderer(WindowConfig const &info, Config const &settings)
    : config_{settings}
    , instance_context_{info}
    , device_context_{instance_context_.get_instance(), instance_context_.get_surface()}
    , allocator_{std::make_shared<MemoryListAllocator>(device_context_.get_device(), device_context_.get_physical_device())}
    , swapchain_context_{device_context_.get_device(), allocator_, get_swapchain_context_info()}
    , swapchain_presenter_{device_context_.get_device(), device_context_.get_graphics_queue(), device_context_.get_present_queue()} {
    auto window_ctx = WindowContext::get_window_context(instance_context_.get_window());
    window_ctx->set_resize_callback(std::bind(&GraphicsRenderer::on_window_resized, this, std::placeholders::_1, std::placeholders::_2));
}

GraphicsRenderer::~GraphicsRenderer() {
    wait_device();
}

void GraphicsRenderer::run() {
    int width, height;
    glfwGetWindowSize(instance_context_.get_window(), &width, &height);
    on_window_resized(width, height);
    while (!glfwWindowShouldClose(instance_context_.get_window())) {
        glfwPollEvents();
        // draw frame
        swapchain_presenter_.submit_and_present(swapchain_context_.get_swapchain(), swapchain_context_.get_image());
    }
    wait_device();
}

void GraphicsRenderer::set_cursor_callback(WindowConfig::cursor_t const &callback) {
    WindowContext::get_window_context(instance_context_.get_window())->set_cursor_callback(callback);
}

void GraphicsRenderer::set_keyboard_callback(WindowConfig::keyboard_t const &callback) {
    WindowContext::get_window_context(instance_context_.get_window())->set_keyboard_callback(callback);
}

void GraphicsRenderer::set_mouse_button_callback(WindowConfig::mouse_button_t const &callback) {
    WindowContext::get_window_context(instance_context_.get_window())->set_mouse_button_callback(callback);
}

void GraphicsRenderer::set_mouse_scroll_callback(WindowConfig::mouse_scroll_t const &callback) {
    WindowContext::get_window_context(instance_context_.get_window())->set_mouse_scroll_callback(callback);
}

void GraphicsRenderer::on_window_resized(int width, int height) {
    wait_device();
    if (width == 0 || height == 0) {
        // there is no need to recreate swapchain and set command buffers
        return;
    }
    VkSurfaceCapabilitiesKHR surface_capabilities = get_surface_capabilities();
    swapchain_context_.update_extent(surface_capabilities.currentExtent, device_context_.get_graphics_queue());
    if (context_changed_) {
        context_changed_(Context{
            .render_pass = swapchain_context_.get_render_pass(),
            .surface_extent = surface_capabilities.currentExtent,
            .images_count = swapchain_context_.get_images_count(),
        });
    }
    set_command_buffers();
}

void GraphicsRenderer::update_render_pass() {
    wait_device();
    int width, height;
    glfwGetFramebufferSize(instance_context_.get_window(), &width, &height);
    if (width == 0 || height == 0) {
        return;
    }
    set_command_buffers();
}

void GraphicsRenderer::set_command_buffers() {
    if (update_command_) {
        uint32_t index = 0;
        for (auto &renderer : swapchain_context_.get_image_renderers()) {
            update_command_(renderer.begin_render_pass(), index++);
            renderer.end_render_pass();
        }
    } else {
        for (auto &renderer : swapchain_context_.get_image_renderers()) {
            renderer.begin_render_pass();
            renderer.end_render_pass();
        }
    }
}

VkSurfaceCapabilitiesKHR GraphicsRenderer::get_surface_capabilities() const {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_context_.get_physical_device(), instance_context_.get_surface(), &capabilities);
    return capabilities;
}

void GraphicsRenderer::wait_device() const {
    vk_assert(vkDeviceWaitIdle(device_context_.get_device().get()), "Failed to wait idles.");
}

SwapchainContext::Info GraphicsRenderer::get_swapchain_context_info() const {
    VkSurfaceCapabilitiesKHR surface_capabilities = get_surface_capabilities();
    SwapchainContext::Info info;
    info.swapchain_info = {
        .surface = instance_context_.get_surface(),
        .surface_format = get_supported_surface_format(),
        .present_mode = get_supported_present_mode(),
        .pre_transform = surface_capabilities.currentTransform,
        .composite_alpha = config_.composite_alpha,
        .image_usage = config_.image_usage,
        .images_count = std::min(surface_capabilities.maxImageCount, surface_capabilities.minImageCount + 1),
        .graphics_qfm = device_context_.get_graphics_qfm(),
        .present_qfm = device_context_.get_present_qfm(),
    };
    if (config_.depth_buffering) {
        info.depth_info = {
            .depth_tiling = VK_IMAGE_TILING_OPTIMAL,
            .depth_format = get_supported_depth_format(),
        };
    }
    return info;
}

VkPresentModeKHR GraphicsRenderer::get_supported_present_mode() const {
    uint32_t modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_context_.get_physical_device(),
                                              instance_context_.get_surface(),
                                              &modes_count,
                                              nullptr);
    std::vector<VkPresentModeKHR> present_modes(modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_context_.get_physical_device(),
                                              instance_context_.get_surface(),
                                              &modes_count,
                                              present_modes.data());
    if (present_modes.empty()) {
        raise_error("There are no supported surface present modes.");
    }
    if (contains(present_modes, config_.desired_present_mode)) {
        return config_.desired_present_mode;
    }
    if (contains(present_modes, config_.default_present_mode)) {
        return config_.default_present_mode;
    }
    return present_modes.front();
}

VkSurfaceFormatKHR GraphicsRenderer::get_supported_surface_format() const {
    uint32_t formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_context_.get_physical_device(), instance_context_.get_surface(), &formats_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_context_.get_physical_device(),
                                         instance_context_.get_surface(),
                                         &formats_count,
                                         surface_formats.data());
    if (surface_formats.empty()) {
        raise_error("There are no supported surface formats.");
    }
    if (!contains(surface_formats, config_.surface_format)) {
        return config_.surface_format;
    }
    return surface_formats.front();
}

VkFormat GraphicsRenderer::get_supported_depth_format() const {
    std::array formats{
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };
    VkFormatFeatureFlags feature = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkPhysicalDevice device = device_context_.get_physical_device();
    auto iter = std::find_if(formats.begin(), formats.end(), [feature, tiling, device](VkFormat format) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device, format, &properties);
        switch (tiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return (properties.optimalTilingFeatures & feature) == feature;
        case VK_IMAGE_TILING_LINEAR:
            return (properties.linearTilingFeatures & feature) == feature;
        default:
            return false;
        }
    });
    if (iter == formats.end()) {
        raise_error("Failed to find supported depth format: tiling={}.", tiling_to_str(tiling));
    }
    info_println("Found depth format {}", static_cast<int>(*iter));
    return *iter;
}

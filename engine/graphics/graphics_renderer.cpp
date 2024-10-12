#include "graphics_renderer.hpp"

#include "graphics_error.hpp"
#include "window_context.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>

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

} // namespace

GraphicsRenderer::GraphicsRenderer(WindowConfig const &info)
    : GraphicsRenderer(info, Config{}) {
}

GraphicsRenderer::GraphicsRenderer(WindowConfig const &info, Config const &settings)
    : config_{settings}
    , instance_context_{info}
    , device_context_{instance_context_.get_instance(), instance_context_.get_surface()}
    , swapchain_context_{device_context_.get_device(), get_swapchain_context_info()}
    , swapchain_presenter_{device_context_.get_device(), device_context_.get_graphics_qfm(), device_context_.get_present_qfm()} {
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
    swapchain_context_.update_extent(surface_capabilities.currentExtent);
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

SwapchainContext::Config GraphicsRenderer::get_swapchain_context_info() const {
    VkSurfaceCapabilitiesKHR surface_capabilities = get_surface_capabilities();
    SwapchainContext::Config info;
    info.surface = instance_context_.get_surface();
    info.surface_format = get_supported_surface_format();
    info.present_mode = get_supported_present_mode();
    info.pre_transform = surface_capabilities.currentTransform;
    info.composite_alpha = config_.composite_alpha;
    info.image_usage = config_.image_usage;
    info.images_count = std::min(surface_capabilities.maxImageCount, surface_capabilities.minImageCount + 1);
    info.graphics_qfm = device_context_.get_graphics_qfm();
    info.present_qfm = device_context_.get_present_qfm();
    return info;
}

VkPresentModeKHR GraphicsRenderer::get_supported_present_mode() const {
    uint32_t modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_context_.get_physical_device(), instance_context_.get_surface(), &modes_count, nullptr);
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

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

graphics_renderer::graphics_renderer(window_info const &info)
    : graphics_renderer(info, settings{}) {
}

graphics_renderer::graphics_renderer(window_info const &info, settings const &settings)
    : settings_{settings}
    , instance_ctx_{info}
    , device_ctx_{instance_ctx_.get_instance(), instance_ctx_.get_surface()}
    , swapchain_ctx_{device_ctx_.get_device(), get_swapchain_context_info()}
    , swapchain_presenter_{device_ctx_.get_device(), device_ctx_.get_graphics_queue(), device_ctx_.get_present_queue()} {
    auto window_ctx = window_context::get_window_context(instance_ctx_.get_window());
    window_ctx->set_resize_callback(std::bind(&graphics_renderer::on_window_resized, this, std::placeholders::_1, std::placeholders::_2));
}

graphics_renderer::~graphics_renderer() {
    wait_device();
}

void graphics_renderer::run() {
    int width, height;
    glfwGetWindowSize(instance_ctx_.get_window(), &width, &height);
    on_window_resized(width, height);
    while (!glfwWindowShouldClose(instance_ctx_.get_window())) {
        glfwPollEvents();
        // draw frame
        swapchain_presenter_.submit_and_present(swapchain_ctx_);
    }
}

void graphics_renderer::set_rendering_changed_callback(rendering_changed_callback_t const &callback) {
    rendering_changed_ = callback;
}

void graphics_renderer::set_cursor_callback(cursor_callback_t const &callback) {
    auto window_ctx = window_context::get_window_context(instance_ctx_.get_window());
    window_ctx->set_cursor_callback(callback);
}

void graphics_renderer::set_keyboard_callback(keyboard_callback_t const &callback) {
    auto window_ctx = window_context::get_window_context(instance_ctx_.get_window());
    window_ctx->set_keyboard_callback(callback);
}

void graphics_renderer::set_mouse_button_callback(mouse_button_callback_t const &callback) {
    auto window_ctx = window_context::get_window_context(instance_ctx_.get_window());
    window_ctx->set_mouse_button_callback(callback);
}

void graphics_renderer::set_mouse_scroll_callback(mouse_scroll_callback_t const &callback) {
    auto window_ctx = window_context::get_window_context(instance_ctx_.get_window());
    window_ctx->set_mouse_scroll_callback(callback);
}

void graphics_renderer::set_pipeline_provider(std::shared_ptr<pipeline_provider> provider) {
    pipeline_provider_.swap(provider);
    pipeline_provider_->set_update_rendering(std::bind(&graphics_renderer::update_render_pass, this));
}

void graphics_renderer::on_window_resized(int width, int height) {
    wait_device();
    if (width == 0 || height == 0) {
        // there is no need to recreate swapchain and set command buffers
        return;
    }
    VkSurfaceCapabilitiesKHR surface_capabilities = get_surface_capabilities();
    swapchain_ctx_.update_extent(surface_capabilities.currentExtent);
    if (rendering_changed_) {
        rendering_changed_(rendering_info{
            .render_pass = swapchain_ctx_.get_render_pass(),
            .extent = surface_capabilities.currentExtent,
        });
    }
    set_command_buffers();
}

void graphics_renderer::update_render_pass() {
    wait_device();
    int width, height;
    glfwGetFramebufferSize(instance_ctx_.get_window(), &width, &height);
    if (width == 0 || height == 0) {
        return;
    }
    set_command_buffers();
}

void graphics_renderer::set_command_buffers() {
    if (pipeline_provider_) {
        for (auto &renderer : swapchain_ctx_.get_image_renderers()) {
            pipeline_provider_->update_command_buffer(renderer.begin_render_pass());
            renderer.end_render_pass();
        }
    } else {
        for (auto &renderer : swapchain_ctx_.get_image_renderers()) {
            renderer.begin_render_pass();
            renderer.end_render_pass();
        }
    }
}

VkSurfaceCapabilitiesKHR graphics_renderer::get_surface_capabilities() const {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_ctx_.get_physical_device(), instance_ctx_.get_surface(), &capabilities);
    return capabilities;
}

void graphics_renderer::wait_device() const {
    vk_assert(vkDeviceWaitIdle(device_ctx_.get_device().get()), "Failed to wait idles.");
}

swapchain_context::context_info graphics_renderer::get_swapchain_context_info() const {
    VkSurfaceCapabilitiesKHR surface_capabilities = get_surface_capabilities();
    swapchain_context::context_info info;
    info.surface = instance_ctx_.get_surface();
    info.surface_format = get_supported_surface_format();
    info.present_mode = get_supported_present_mode();
    info.pre_transform = surface_capabilities.currentTransform;
    info.composite_alpha = settings_.composite_alpha;
    info.image_usage = settings_.image_usage;
    info.images_count = std::min(surface_capabilities.maxImageCount, surface_capabilities.minImageCount + 1);
    info.graphics_qfm = device_ctx_.get_graphics_qfm();
    info.present_qfm = device_ctx_.get_present_qfm();
    return info;
}

VkPresentModeKHR graphics_renderer::get_supported_present_mode() const {
    uint32_t modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_ctx_.get_physical_device(), instance_ctx_.get_surface(), &modes_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_ctx_.get_physical_device(),
                                              instance_ctx_.get_surface(),
                                              &modes_count,
                                              present_modes.data());
    if (present_modes.empty()) {
        raise_error("There are no supported surface present modes.");
    }
    if (contains(present_modes, settings_.desired_present_mode)) {
        return settings_.desired_present_mode;
    }
    if (contains(present_modes, settings_.default_present_mode)) {
        return settings_.default_present_mode;
    }
    return present_modes.front();
}

VkSurfaceFormatKHR graphics_renderer::get_supported_surface_format() const {
    uint32_t formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_ctx_.get_physical_device(), instance_ctx_.get_surface(), &formats_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_ctx_.get_physical_device(),
                                         instance_ctx_.get_surface(),
                                         &formats_count,
                                         surface_formats.data());
    if (surface_formats.empty()) {
        raise_error("There are no supported surface formats.");
    }
    if (!contains(surface_formats, settings_.surface_format)) {
        return settings_.surface_format;
    }
    return surface_formats.front();
}

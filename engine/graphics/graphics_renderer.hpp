#pragma once

#include "device_context.hpp"
#include "instance_context.hpp"
#include "swapchain_context.hpp"
#include "swapchain_presenter.hpp"
#include "window_config.hpp"

class GraphicsRenderer {
  public:
    struct Config {
        VkSurfaceFormatKHR surface_format{
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        };
        VkPresentModeKHR desired_present_mode{VK_PRESENT_MODE_MAILBOX_KHR};
        VkPresentModeKHR default_present_mode{VK_PRESENT_MODE_FIFO_KHR};
        VkImageUsageFlags image_usage{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
        VkCompositeAlphaFlagBitsKHR composite_alpha{VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};
    };

    struct Context {
        VkRenderPass render_pass;
        VkExtent2D surface_extent;
        uint32_t images_count;
    };

    using context_changed_t = std::function<void(Context const &)>;
    using update_command_t = std::function<void(VkCommandBuffer, size_t)>;

  private:
    Config config_;
    InstanceContext instance_context_;
    DeviceContext device_context_;
    SwapchainContext swapchain_context_;
    SwapchainPresenter swapchain_presenter_;
    context_changed_t context_changed_;
    update_command_t update_command_;

  public:
    explicit GraphicsRenderer(WindowConfig const &info);

    GraphicsRenderer(WindowConfig const &info, Config const &settings);

    ~GraphicsRenderer();

    void run();

    DeviceContext const &get_device_context() const {
        return device_context_;
    }

    void set_context_changed_callback(context_changed_t const &callback) {
        context_changed_ = callback;
    }

    void set_update_command_callback(update_command_t const &callback) {
        update_command_ = callback;
    }

    void set_update_frame_callback(SwapchainPresenter::update_frame_t const &callback) {
        swapchain_presenter_.set_update_frame_callback(callback);
    }

    void set_cursor_callback(WindowConfig::cursor_t const &callback);

    void set_keyboard_callback(WindowConfig::keyboard_t const &callback);

    void set_mouse_button_callback(WindowConfig::mouse_button_t const &callback);

    void set_mouse_scroll_callback(WindowConfig::mouse_scroll_t const &callback);

    void update_render_pass();

  private:
    void on_window_resized(int width, int height);

    void set_command_buffers();

    VkSurfaceCapabilitiesKHR get_surface_capabilities() const;

    void wait_device() const;

    SwapchainContext::Config get_swapchain_context_info() const;

    VkPresentModeKHR get_supported_present_mode() const;

    VkSurfaceFormatKHR get_supported_surface_format() const;
};
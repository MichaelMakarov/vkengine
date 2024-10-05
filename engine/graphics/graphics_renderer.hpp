#pragma once

#include "device_context.hpp"
#include "instance_context.hpp"
#include "pipeline_provider.hpp"
#include "swapchain_context.hpp"
#include "swapchain_presenter.hpp"
#include "window_info.hpp"

struct rendering_info {
    VkRenderPass render_pass;
    VkExtent2D extent;
};

using rendering_changed_callback_t = std::function<void(rendering_info const &)>;

class GraphicsRenderer {
  public:
    struct Settings {
        VkSurfaceFormatKHR surface_format{
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        };
        VkPresentModeKHR desired_present_mode{VK_PRESENT_MODE_MAILBOX_KHR};
        VkPresentModeKHR default_present_mode{VK_PRESENT_MODE_FIFO_KHR};
        VkImageUsageFlags image_usage{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
        VkCompositeAlphaFlagBitsKHR composite_alpha{VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};
    };

  private:
    Settings settings_;
    InstanceContext instance_ctx_;
    DeviceContext device_ctx_;
    SwapchainContext swapchain_ctx_;
    SwapchainPresenter swapchain_presenter_;
    rendering_changed_callback_t rendering_changed_;
    std::shared_ptr<PipelineProvider> pipeline_provider_;

  public:
    explicit GraphicsRenderer(WindowConfig const &info);

    GraphicsRenderer(WindowConfig const &info, Settings const &settings);

    ~GraphicsRenderer();

    void run();

    DeviceContext const &get_device_context() const {
        return device_ctx_;
    }

    void set_rendering_changed_callback(rendering_changed_callback_t const &callback);

    void set_cursor_callback(cursor_callback_t const &callback);

    void set_keyboard_callback(keyboard_callback_t const &callback);

    void set_mouse_button_callback(mouse_button_callback_t const &callback);

    void set_mouse_scroll_callback(mouse_scroll_callback_t const &callback);

    void set_pipeline_provider(std::shared_ptr<PipelineProvider> provider);

  private:
    void on_window_resized(int width, int height);

    void update_render_pass();

    void set_command_buffers();

    VkSurfaceCapabilitiesKHR get_surface_capabilities() const;

    void wait_device() const;

    SwapchainContext::config get_swapchain_context_info() const;

    VkPresentModeKHR get_supported_present_mode() const;

    VkSurfaceFormatKHR get_supported_surface_format() const;
};
#pragma once
#include <vulkan/vulkan.h>
#include <engine.hpp>
#include <error.hpp>
#include <vector>

#define vk_assert(res, msg) \
    if (res != VK_SUCCESS)  \
        RAISE(msg);

template <typename T>
using ptr_value_type = typename std::pointer_traits<T>::element_type;

template <>
struct std::default_delete<GLFWwindow>
{
    void operator()(GLFWwindow *) const;
};

using instance_t = ptr_value_type<VkInstance>;
template <>
struct std::default_delete<instance_t>
{
    void operator()(VkInstance) const;
};

using debug_messenger_t = ptr_value_type<VkDebugUtilsMessengerEXT>;
template <>
struct std::default_delete<debug_messenger_t>
{
    std::shared_ptr<instance_t> instance;
    void operator()(VkDebugUtilsMessengerEXT) const;
};

using surface_t = ptr_value_type<VkSurfaceKHR>;
template <>
struct std::default_delete<surface_t>
{
    std::shared_ptr<instance_t> instance;
    void operator()(VkSurfaceKHR) const;
};

using device_t = ptr_value_type<VkDevice>;
template <>
struct std::default_delete<device_t>
{
    void operator()(VkDevice) const;
};

using command_pool_t = ptr_value_type<VkCommandPool>;
template <>
struct std::default_delete<command_pool_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkCommandPool) const;
};

using render_pass_t = ptr_value_type<VkRenderPass>;
template <>
struct std::default_delete<render_pass_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkRenderPass) const;
};

using pipeline_t = ptr_value_type<VkPipeline>;
template <>
struct std::default_delete<pipeline_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkPipeline) const;
};

using swapchain_t = ptr_value_type<VkSwapchainKHR>;
template <>
struct std::default_delete<swapchain_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkSwapchainKHR) const;
};

using pipeline_layout_t = ptr_value_type<VkPipelineLayout>;
template <>
struct std::default_delete<pipeline_layout_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkPipelineLayout) const;
};

using image_view_t = ptr_value_type<VkImageView>;
template <>
struct std::default_delete<image_view_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkImageView) const;
};

using framebuffer_t = ptr_value_type<VkFramebuffer>;
template <>
struct std::default_delete<framebuffer_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkFramebuffer) const;
};

using fence_t = ptr_value_type<VkFence>;
template <>
struct std::default_delete<fence_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkFence) const;
};

using semaphore_t = ptr_value_type<VkSemaphore>;
template <>
struct std::default_delete<semaphore_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkSemaphore) const;
};

using shader_module_t = ptr_value_type<VkShaderModule>;
template <>
struct std::default_delete<shader_module_t>
{
    std::shared_ptr<device_t> device;
    void operator()(VkShaderModule) const;
};

struct device_properties
{
    uint32_t graphics_qfm;
    uint32_t present_qfm;
    uint32_t compute_qfm;
    uint32_t transfer_qfm;

    device_properties() = default;
    device_properties(VkPhysicalDevice device, VkSurfaceKHR surface);
    device_properties(device_properties const &) = default;
    device_properties &operator=(device_properties const &) = default;
};

struct pipeline_drawer
{
    virtual ~pipeline_drawer() = default;
    virtual void draw_command(VkCommandBuffer) = 0;
    virtual void update_rendering(std::shared_ptr<device_t> const &, VkRenderPass, VkExtent2D) = 0;
};

struct copy_descriptor
{
    VkBuffer srcbuf, dstbuf;
    uint32_t size;
    uint32_t srcoffset, dstoffset;
};

class copy_manager
{
    std::shared_ptr<device_t> _device;
    std::shared_ptr<command_pool_t> _cmdpool;
    std::shared_ptr<fence_t> _fence;
    VkQueue _queue;

public:
    copy_manager(std::shared_ptr<device_t> const &device, uint32_t transfer_qfm);
    void copy(copy_descriptor const *descriptors, std::size_t descriptors_count);
};

class graphics_renderer
{
    VkPhysicalDevice _physdevice;
    device_properties _props;
    VkExtent2D _extent;
    VkQueue _graphics_queue;
    VkQueue _present_queue;
    VkQueue _transfer_queue;
    std::size_t _frame_index{};

    std::shared_ptr<GLFWwindow> _wnd;
    std::shared_ptr<instance_t> _instance;
    std::shared_ptr<debug_messenger_t> _messenger;
    std::shared_ptr<surface_t> _surface;
    std::shared_ptr<device_t> _device;

    std::unique_ptr<swapchain_t> _swapchain;
    std::unique_ptr<render_pass_t> _render_pass;
    std::unique_ptr<command_pool_t> _frame_cmdpool;
    std::vector<std::shared_ptr<image_view_t>> _image_views;
    std::vector<std::shared_ptr<framebuffer_t>> _framebuffers;
    std::vector<std::unique_ptr<fence_t>> _frame_sync_fences;
    std::vector<std::unique_ptr<semaphore_t>> _frame_submit_semaphores;
    std::vector<std::unique_ptr<semaphore_t>> _frame_present_semaphores;
    std::vector<VkCommandBuffer> _frame_cmdbuffers;

    std::vector<std::unique_ptr<pipeline_drawer>> _pipeline_drawers;

    std::unique_ptr<command_pool_t> _copy_cmdpool;
    std::unique_ptr<fence_t> _copy_sync_fence;
    std::vector<VkCommandBuffer> _copy_cmdbuffers;
    std::vector<copy_descriptor> _copy_descriptors;

public:
    graphics_renderer(std::shared_ptr<GLFWwindow> const &wnd);
    ~graphics_renderer();
    void run();
    void on_window_resized(int w, int h);
    void add_content(std::shared_ptr<content_provider> const &provider);

private:
    void create_core();
    void create_swapchain();
    void create_pipeline();
    void draw_frame();
    void update_resources();
    void set_draw_commands();
    void copy_resources();
    void clear_framebuffers();
    void stop_drawing();
};
#include <renderer.hpp>
#include <iostream>

void std::default_delete<instance_t>::operator()(VkInstance instance) const
{
    std::cout << "Instance deletion." << std::endl;
    vkDestroyInstance(instance, nullptr);
}

void std::default_delete<debug_messenger_t>::operator()(VkDebugUtilsMessengerEXT messenger) const
{
    std::cout << "Debug utils messenger deletion." << std::endl;
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
    if (func)
    {
        func(instance.get(), messenger, nullptr);
    }
}

void std::default_delete<surface_t>::operator()(VkSurfaceKHR surface) const
{
    std::cout << "Surface deletion." << std::endl;
    vkDestroySurfaceKHR(instance.get(), surface, nullptr);
}

void std::default_delete<device_t>::operator()(VkDevice device) const
{
    std::cout << "Device deletion." << std::endl;
    vkDestroyDevice(device, nullptr);
}

void std::default_delete<command_pool_t>::operator()(VkCommandPool pool) const
{
    std::cout << "Command pool deletion." << std::endl;
    vkDestroyCommandPool(device.get(), pool, nullptr);
}

void std::default_delete<pipeline_t>::operator()(VkPipeline pipeline) const
{
    std::cout << "Pipeline deletion." << std::endl;
    vkDestroyPipeline(device.get(), pipeline, nullptr);
}

void std::default_delete<swapchain_t>::operator()(VkSwapchainKHR swapchain) const
{
    std::cout << "Swapchain deletion." << std::endl;
    vkDestroySwapchainKHR(device.get(), swapchain, nullptr);
}

void std::default_delete<render_pass_t>::operator()(VkRenderPass render_pass) const
{
    std::cout << "Render pass deletion." << std::endl;
    vkDestroyRenderPass(device.get(), render_pass, nullptr);
}

void std::default_delete<pipeline_layout_t>::operator()(VkPipelineLayout pipeline_layout) const
{
    std::cout << "Pipeline layout deletion." << std::endl;
    vkDestroyPipelineLayout(device.get(), pipeline_layout, nullptr);
}

void std::default_delete<fence_t>::operator()(VkFence fence) const
{
    std::cout << "Fence deletion." << std::endl;
    vkDestroyFence(device.get(), fence, nullptr);
}

void std::default_delete<semaphore_t>::operator()(VkSemaphore semaphore) const
{
    std::cout << "Semaphore deletion." << std::endl;
    vkDestroySemaphore(device.get(), semaphore, nullptr);
}

void std::default_delete<image_view_t>::operator()(VkImageView image_view) const
{
    std::cout << "Image view deletion." << std::endl;
    vkDestroyImageView(device.get(), image_view, nullptr);
}

void std::default_delete<framebuffer_t>::operator()(VkFramebuffer framebuffer) const
{
    std::cout << "Framebuffer deletion." << std::endl;
    vkDestroyFramebuffer(device.get(), framebuffer, nullptr);
}

void std::default_delete<shader_module_t>::operator()(VkShaderModule shader_module) const
{
    std::cout << "Shader module deletion." << std::endl;
    vkDestroyShaderModule(device.get(), shader_module, nullptr);
}
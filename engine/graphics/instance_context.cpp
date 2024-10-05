#include "instance_context.hpp"

#include "graphics_manager.hpp"
#include "window_context.hpp"

namespace {

    std::vector<char const *> get_glfw_extensions() {
        unsigned count;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&count);
        return std::vector<char const *>(glfw_extensions, glfw_extensions + count);
    }

} // namespace

InstanceContext::InstanceContext(WindowConfig const &info) {
    auto extensions = get_glfw_extensions();
    std::vector<char const *> layers;
#if defined DEBUG_APP
    extensions.push_back("VK_EXT_debug_utils");
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    instance_ = GraphicsManager::make_instance("vk_renderer", extensions, layers);
#if defined DEBUG_APP
    debug_messenger_ = make_debug_messenger(instance_);
#endif
    window_ = WindowContext::make_window(info);
    surface_ = GraphicsManager::make_surface(instance_, window_.get());
}
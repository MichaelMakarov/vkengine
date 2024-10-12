#pragma once

#include "graphics_types.hpp"

#include "window_config.hpp"

#include <GLFW/glfw3.h>

class InstanceContext {
    shared_ptr_of<VkInstance> instance_;
    unique_ptr_of<VkDebugUtilsMessengerEXT> debug_messenger_;
    unique_ptr_of<GLFWwindow *> window_;
    unique_ptr_of<VkSurfaceKHR> surface_;

  public:
    explicit InstanceContext(WindowConfig const &info);

    VkInstance get_instance() const {
        return instance_.get();
    }

    VkSurfaceKHR get_surface() const {
        return surface_.get();
    }

    GLFWwindow *get_window() const {
        return window_.get();
    }
};
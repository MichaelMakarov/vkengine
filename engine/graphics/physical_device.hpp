#pragma once

#include "graphics_types.hpp"

#include "queue_family.hpp"

#include <vector>
#include <string>

struct physical_device {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    std::vector<queue_family> graphics_qfms;
    std::vector<queue_family> transfer_qfms;
    std::vector<queue_family> compute_qfms;

    static std::vector<physical_device> get_physical_devices(VkInstance instance, VkSurfaceKHR surface);

    std::string get_name() const;
};

#pragma once

#include "graphics_types.hpp"

#include "queue_family.hpp"

#include <vector>
#include <string>

struct PhysicalDevice {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    std::vector<QueueFamily> graphics_qfms;
    std::vector<QueueFamily> transfer_qfms;
    std::vector<QueueFamily> compute_qfms;

    static std::vector<PhysicalDevice> get_physical_devices(VkInstance instance, VkSurfaceKHR surface);

    std::string get_name() const;
};

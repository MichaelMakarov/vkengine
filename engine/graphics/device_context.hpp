#pragma once

#include "graphics_types.hpp"

struct QueueFamily {
    VkQueueFamilyProperties properties;
    uint32_t index;
};

class DeviceContext {
    VkPhysicalDevice phys_device_;
    VkPhysicalDeviceProperties properties_;
    VkPhysicalDeviceFeatures features_;
    QueueFamily graphics_qfm_;
    QueueFamily present_qfm_;
    QueueFamily compute_qfm_;
    QueueFamily transfer_qfm_;
    shared_ptr_of<VkDevice> device_;

  public:
    DeviceContext(VkInstance instance, VkSurfaceKHR surface);

    VkPhysicalDevice get_physical_device() const {
        return phys_device_;
    }

    shared_ptr_of<VkDevice> const &get_device() const {
        return device_;
    }

    uint32_t get_graphics_qfm() const {
        return graphics_qfm_.index;
    }

    uint32_t get_present_qfm() const {
        return present_qfm_.index;
    }

    uint32_t get_compute_qfm() const {
        return compute_qfm_.index;
    }

    uint32_t get_transfer_qfm() const {
        return transfer_qfm_.index;
    }
};
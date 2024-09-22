#pragma once

#include "graphics_types.hpp"

#include "queue_family.hpp"

class device_context {
    VkPhysicalDevice phys_device_;
    shared_ptr_of<VkDevice> device_;
    queue_family graphics_qfm_;
    queue_family present_qfm_;
    queue_family compute_qfm_;
    queue_family transfer_qfm_;
    VkQueue graphics_queue_{nullptr};
    VkQueue present_queue_{nullptr};
    VkQueue compute_queue_{nullptr};
    VkQueue transfer_queue_{nullptr};

  public:
    device_context(VkInstance instance, VkSurfaceKHR surface);

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

    VkQueue get_graphics_queue() const {
        return graphics_queue_;
    }

    VkQueue get_present_queue() const {
        return present_queue_;
    }

    VkQueue get_compute_queue() const {
        return compute_queue_;
    }

    VkQueue get_transfer_queue() const {
        return transfer_queue_;
    }
};
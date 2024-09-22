#include "device_context.hpp"

#include "graphics_manager.hpp"
#include "physical_device.hpp"
#include "graphics_error.hpp"

#include <algorithm>
#include <set>
#include <vector>

namespace {

    bool is_surface_supported(VkPhysicalDevice phys_device, VkSurfaceKHR surface, queue_family const &qfm) {
        VkBool32 surface_supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, qfm.index, surface, &surface_supported);
        return surface_supported != VK_FALSE;
    }

    size_t physical_device_rate(VkSurfaceKHR surface, physical_device const &device) {
        size_t total_score{1};
        if (device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            total_score += 100;
        }
        if (device.features.geometryShader == VK_FALSE) {
            return 0;
        }

        size_t supported_graphics_qfms = 0;
        for (queue_family const &qfm : device.graphics_qfms) {
            if (is_surface_supported(device.device, surface, qfm)) {
                supported_graphics_qfms++;
            }
        }
        if (supported_graphics_qfms == 0) {
            return 0;
        }

        return total_score;
    }

    physical_device get_best_physical_device(VkInstance instance, VkSurfaceKHR surface) {
        auto phys_devices = physical_device::get_physical_devices(instance, surface);
        if (phys_devices.empty()) {
            raise_error("There are no available physical devices.");
        }
        std::sort(phys_devices.begin(), phys_devices.end(), [surface](physical_device const &left, physical_device const &right) {
            return physical_device_rate(surface, left) > physical_device_rate(surface, right);
        });
        if (phys_devices.size() > 1) {
            debug_println("Found {} physical devices (desc. order by rate score):", phys_devices.size());
            for (size_t i = 0; i < phys_devices.size(); ++i) {
                debug_println("{}) {}", i + 1, phys_devices[i].get_name());
            }
        } else {
            debug_println("Found {}", phys_devices.front().get_name());
        }
        if (physical_device_rate(surface, phys_devices.front()) == 0) {
            raise_error("There is no suitable physical device.");
        }
        return phys_devices.front();
    }

    VkQueue get_device_queue(VkDevice device, uint32_t qfm_index, uint32_t queue_index) {
        VkQueue queue{nullptr};
        vkGetDeviceQueue(device, qfm_index, queue_index, &queue);
        if (queue == nullptr) {
            raise_error("vkGetDeviceQueue() return nullptr for queue_family_index=%u, queue_index=%u", qfm_index, queue_index);
        }
        return queue;
    }

    class qfm_find_helper {
        std::set<uint32_t> qfm_indices_;

      public:
        void add_index(uint32_t qfm_index) {
            qfm_indices_.insert(qfm_index);
        }

        template <typename iterator>
        iterator find_free(iterator begin, iterator end) {
            return std::find_if(begin, end, [this](queue_family const &qfm) { return !qfm_indices_.contains(qfm.index); });
        }
    };

} // namespace

device_context::device_context(VkInstance instance, VkSurfaceKHR surface) {
    physical_device device = get_best_physical_device(instance, surface);
    phys_device_ = device.device;
    qfm_find_helper find_helper;

    if (device.graphics_qfms.empty()) {
        raise_error("There is no graphics queue families.");
    }
    // find appropriate graphics queue family
    auto iter = std::find_if(device.graphics_qfms.begin(),
                             device.graphics_qfms.end(),
                             std::bind(&is_surface_supported, phys_device_, surface, std::placeholders::_1));
    if (iter == device.graphics_qfms.end()) {
        raise_error("There is no graphics queue family indices supported by surface.");
    }
    graphics_qfm_ = *iter;
    find_helper.add_index(graphics_qfm_.index);
    debug_println("Found graphics queue family {}", graphics_qfm_.index);

    // find present queue family
    if (device.graphics_qfms.size() > 1) {
        iter = find_helper.find_free(device.graphics_qfms.begin(), device.graphics_qfms.end());
        if (iter == device.graphics_qfms.end()) {
            present_qfm_ = graphics_qfm_;
            debug_println("Select same present and graphics queue families");
        } else {
            present_qfm_ = *iter;
            debug_println("Found present queue family {}", present_qfm_.index);
            find_helper.add_index(present_qfm_.index);
        }
    } else {
        present_qfm_ = graphics_qfm_;
    }

    // find compute queue family
    if (device.compute_qfms.empty()) {
        raise_error("There are no compute queue families");
    }
    iter = find_helper.find_free(device.compute_qfms.begin(), device.compute_qfms.end());
    if (iter == device.compute_qfms.end()) {
        // there must be at list one compute queue family
        compute_qfm_ = device.compute_qfms.front();
        debug_println("Select same graphics and compute queue families");
    } else {
        compute_qfm_ = *iter;
        debug_println("Found compute queue family {}", compute_qfm_.index);
        find_helper.add_index(compute_qfm_.index);
    }

    // find transfer queue family
    if (device.transfer_qfms.empty()) {
        raise_error("There are no transfer queue families");
    }
    iter = find_helper.find_free(device.transfer_qfms.begin(), device.transfer_qfms.end());
    if (iter == device.transfer_qfms.end()) {
        // there must be at list one transfer queue family that is usually same as graphics one
        transfer_qfm_ = device.transfer_qfms.front();
        debug_println("Select same graphics and transfer queue families");
    } else {
        transfer_qfm_ = *iter;
        debug_println("Found transfer queue family {}", transfer_qfm_.index);
        find_helper.add_index(transfer_qfm_.index);
    }

    std::vector<float> queue_priorities{1.0f};
    std::vector<VkDeviceQueueCreateInfo> queue_infos;
    queue_infos.reserve(4);
    queue_infos.push_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphics_qfm_.index,
        .queueCount = static_cast<uint32_t>(queue_priorities.size()),
        .pQueuePriorities = queue_priorities.data(),
    });
    if (graphics_qfm_.index != present_qfm_.index) {
        queue_infos.push_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = present_qfm_.index,
            .queueCount = static_cast<uint32_t>(queue_priorities.size()),
            .pQueuePriorities = queue_priorities.data(),
        });
    }
    if (graphics_qfm_.index != compute_qfm_.index) {
        queue_infos.push_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = compute_qfm_.index,
            .queueCount = static_cast<uint32_t>(queue_priorities.size()),
            .pQueuePriorities = queue_priorities.data(),
        });
    }
    if (graphics_qfm_.index != transfer_qfm_.index) {
        queue_infos.push_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = transfer_qfm_.index,
            .queueCount = static_cast<uint32_t>(queue_priorities.size()),
            .pQueuePriorities = queue_priorities.data(),
        });
    }
    std::vector<char const *> extension_names{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    device_ = graphics_manager::make_device(phys_device_, queue_infos, extension_names);

    graphics_queue_ = get_device_queue(device_.get(), graphics_qfm_.index, 0);
    present_queue_ = get_device_queue(device_.get(), present_qfm_.index, 0);
    compute_queue_ = get_device_queue(device_.get(), compute_qfm_.index, 0);
    transfer_queue_ = get_device_queue(device_.get(), transfer_qfm_.index, 0);
}

#include "device_context.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

#include <algorithm>
#include <set>
#include <vector>

namespace {

    char const *device_type_to_str(VkPhysicalDeviceType type) {
        switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "descrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        default:
            return "unknown";
        }
    }

    class PhysicalDevice {
      public:
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        std::vector<QueueFamily> graphics_qfms;
        std::vector<QueueFamily> transfer_qfms;
        std::vector<QueueFamily> compute_qfms;

        PhysicalDevice() = default;

        PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
            : device{device} {
            vkGetPhysicalDeviceProperties(device, &properties);
            vkGetPhysicalDeviceFeatures(device, &features);
            auto qfm_properties = get_queue_family_properties(device);
            graphics_qfms = select_qfm_indices(qfm_properties, VK_QUEUE_GRAPHICS_BIT);
            transfer_qfms = select_qfm_indices(qfm_properties, VK_QUEUE_TRANSFER_BIT);
            compute_qfms = select_qfm_indices(qfm_properties, VK_QUEUE_COMPUTE_BIT);
            std::erase_if(graphics_qfms, [device, surface](QueueFamily const &qfm) { return !is_surface_supported(device, surface, qfm); });
        }

        std::string get_name() const {
            return std::format("{} {} ID={}",
                               device_type_to_str(properties.deviceType),
                               properties.deviceName,
                               properties.deviceID);
        }

        size_t get_rate() const {
            size_t total_score{1};
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                total_score += 100;
            }
            if (features.geometryShader == VK_FALSE) {
                return 0;
            }
            if (graphics_qfms.empty()) {
                return 0;
            }
            if (features.samplerAnisotropy == VK_FALSE) {
                return 0;
            }
            return total_score;
        }

        static PhysicalDevice get_best_physical_device(VkInstance instance, VkSurfaceKHR surface) {
            auto devices = get_physical_devices(instance, surface);
            if (devices.empty()) {
                raise_error("There are no available physical devices.");
            }
            std::sort(devices.begin(), devices.end(), [](PhysicalDevice const &left, PhysicalDevice const &right) {
                return left.get_rate() > right.get_rate();
            });
            if (devices.size() > 1) {
                info_println("Found {} physical devices (desc. order by rate score):", devices.size());
                for (size_t i = 0; i < devices.size(); ++i) {
                    info_println("{}) {}", i + 1, devices[i].get_name());
                }
            } else {
                info_println("Found {}", devices.front().get_name());
            }
            if (devices.front().get_rate() == 0) {
                raise_error("There is no suitable physical device.");
            }
            return devices.front();
        }

      private:
        static std::vector<PhysicalDevice> get_physical_devices(VkInstance instance, VkSurfaceKHR surface) {
            std::uint32_t devices_count;
            vkEnumeratePhysicalDevices(instance, &devices_count, nullptr);
            if (devices_count == 0) {
                return {};
            }
            std::vector<VkPhysicalDevice> devices(devices_count);
            vkEnumeratePhysicalDevices(instance, &devices_count, devices.data());
            std::vector<PhysicalDevice> phys_devices(devices.size());
            std::transform(devices.begin(), devices.end(), phys_devices.begin(), [surface](VkPhysicalDevice device) {
                return PhysicalDevice(device, surface);
            });
            return phys_devices;
        }
        static std::vector<QueueFamily> select_qfm_indices(std::vector<VkQueueFamilyProperties> const &qfm_properties,
                                                           VkQueueFlags queue_flags) {
            std::vector<QueueFamily> qfm_indices;
            qfm_indices.reserve(qfm_properties.size());
            for (uint32_t i = 0; i < qfm_properties.size(); ++i) {
                if (qfm_properties[i].queueFlags & queue_flags) {
                    qfm_indices.push_back(QueueFamily{.properties = qfm_properties[i], .index = i});
                }
            }
            return qfm_indices;
        }

        static std::vector<VkQueueFamilyProperties> get_queue_family_properties(VkPhysicalDevice device) {
            uint32_t qfm_props_count;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &qfm_props_count, nullptr);
            std::vector<VkQueueFamilyProperties> qfm_properties(qfm_props_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &qfm_props_count, qfm_properties.data());
            return qfm_properties;
        }

        static bool is_surface_supported(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamily const &qfm) {
            VkBool32 surface_supported;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, qfm.index, surface, &surface_supported);
            return surface_supported != VK_FALSE;
        }
    };

    class QueueFamilyHelper {
        std::set<uint32_t> qfm_indices_;

      public:
        void add_index(uint32_t qfm_index) {
            qfm_indices_.insert(qfm_index);
        }

        template <typename iterator>
        iterator find_free(iterator begin, iterator end) {
            return std::find_if(begin, end, [this](QueueFamily const &qfm) { return !qfm_indices_.contains(qfm.index); });
        }
    };

} // namespace

DeviceContext::DeviceContext(VkInstance instance, VkSurfaceKHR surface) {
    PhysicalDevice device = PhysicalDevice::get_best_physical_device(instance, surface);
    phys_device_ = device.device;
    properties_ = device.properties;
    features_ = device.features;
    QueueFamilyHelper find_helper;

    if (device.graphics_qfms.empty()) {
        raise_error("There are no graphics queue families.");
    }
    // get graphics queue family
    graphics_qfm_ = device.graphics_qfms.front();
    find_helper.add_index(graphics_qfm_.index);
    info_println("Found graphics queue family {}", graphics_qfm_.index);

    // find present queue family
    if (device.graphics_qfms.size() > 1) {
        auto iter = find_helper.find_free(device.graphics_qfms.begin(), device.graphics_qfms.end());
        if (iter == device.graphics_qfms.end()) {
            present_qfm_ = graphics_qfm_;
            info_println("Select same present and graphics queue families");
        } else {
            present_qfm_ = *iter;
            info_println("Found present queue family {}", present_qfm_.index);
            find_helper.add_index(present_qfm_.index);
        }
    } else {
        present_qfm_ = graphics_qfm_;
    }

    // find compute queue family
    if (device.compute_qfms.empty()) {
        raise_error("There are no compute queue families");
    }
    auto iter = find_helper.find_free(device.compute_qfms.begin(), device.compute_qfms.end());
    if (iter == device.compute_qfms.end()) {
        // there must be at list one compute queue family
        compute_qfm_ = device.compute_qfms.front();
        info_println("Select same graphics and compute queue families");
    } else {
        compute_qfm_ = *iter;
        info_println("Found compute queue family {}", compute_qfm_.index);
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
        info_println("Select same graphics and transfer queue families");
    } else {
        transfer_qfm_ = *iter;
        info_println("Found transfer queue family {}", transfer_qfm_.index);
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
    device_ = GraphicsManager::make_device(phys_device_, queue_infos, extension_names);
}

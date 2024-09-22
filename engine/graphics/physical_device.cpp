#include "physical_device.hpp"

#include <algorithm>
#include <format>

namespace {

    std::vector<queue_family> select_qfm_indices(std::vector<VkQueueFamilyProperties> const &qfm_properties, VkQueueFlags queue_flags) {
        std::vector<queue_family> qfm_indices;
        qfm_indices.reserve(qfm_properties.size());
        for (uint32_t i = 0; i < qfm_properties.size(); ++i) {
            if (qfm_properties[i].queueFlags & queue_flags) {
                qfm_indices.push_back(queue_family{.properties = qfm_properties[i], .index = i});
            }
        }
        return qfm_indices;
    }

    std::vector<VkQueueFamilyProperties> get_queue_family_properties(VkPhysicalDevice device) {
        uint32_t qfm_props_count;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &qfm_props_count, nullptr);
        std::vector<VkQueueFamilyProperties> qfm_properties(qfm_props_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &qfm_props_count, qfm_properties.data());
        return qfm_properties;
    }

    physical_device make_physical_device(VkPhysicalDevice device, VkSurfaceKHR surface) {
        physical_device phys_device;
        phys_device.device = device;
        vkGetPhysicalDeviceProperties(device, &phys_device.properties);
        vkGetPhysicalDeviceFeatures(device, &phys_device.features);

        auto qfm_properties = get_queue_family_properties(device);
        phys_device.graphics_qfms = select_qfm_indices(qfm_properties, VK_QUEUE_GRAPHICS_BIT);

        phys_device.transfer_qfms = select_qfm_indices(qfm_properties, VK_QUEUE_TRANSFER_BIT);
        phys_device.compute_qfms = select_qfm_indices(qfm_properties, VK_QUEUE_COMPUTE_BIT);

        return phys_device;
    }

} // namespace

std::vector<physical_device> physical_device::get_physical_devices(VkInstance instance, VkSurfaceKHR surface) {
    std::uint32_t devices_count;
    vkEnumeratePhysicalDevices(instance, &devices_count, nullptr);
    if (devices_count == 0) {
        return {};
    }
    std::vector<VkPhysicalDevice> devices(devices_count);
    vkEnumeratePhysicalDevices(instance, &devices_count, devices.data());

    std::vector<physical_device> phys_devices(devices.size());
    std::transform(devices.begin(), devices.end(), phys_devices.begin(), std::bind(&make_physical_device, std::placeholders::_1, surface));
    return phys_devices;
}

std::string physical_device::get_name() const {
    return std::format("GPU[{}] {} {}", properties.deviceID, properties.deviceName, static_cast<uint32_t>(properties.deviceType));
}
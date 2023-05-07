#include <renderer.hpp>
#include <GLFW/glfw3.h>
#include <error.hpp>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <array>
#include <set>
#include <algorithm>

std::shared_ptr<instance_t> make_instance()
{
    unsigned count;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&count);
    std::vector<char const *> extensions(glfw_extensions, glfw_extensions + count);
    std::vector<char const *> layers;
#if defined _DEBUG || DEBUG
    extensions.push_back("VK_EXT_debug_utils");
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    VkApplicationInfo appinfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                              .pApplicationName = "",
                              .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                              .pEngineName = "vkengine",
                              .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                              .apiVersion = VK_API_VERSION_1_2};
    VkInstanceCreateInfo instinfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                  .pApplicationInfo = &appinfo,
                                  .enabledLayerCount = static_cast<unsigned>(layers.size()),
                                  .ppEnabledLayerNames = layers.data(),
                                  .enabledExtensionCount = static_cast<unsigned>(extensions.size()),
                                  .ppEnabledExtensionNames = extensions.data()};

    VkInstance inst;
    vk_assert(vkCreateInstance(&instinfo, nullptr, &inst), "Failed to create instance.");
    return std::shared_ptr<instance_t>(inst, std::default_delete<instance_t>{});
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
    VkDebugUtilsMessageTypeFlagsEXT /*msg_type*/,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void * /*user_data*/)
{
    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "validations layer message" << callback_data->pMessage << std::endl;
    }
    return VK_FALSE;
}

std::shared_ptr<debug_messenger_t> make_debug_messenger(std::shared_ptr<instance_t> const &instance)
{
    VkDebugUtilsMessageSeverityFlagsEXT severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT info{.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                                            .messageSeverity = severity,
                                            .messageType = type,
                                            .pfnUserCallback = &debug_callback,
                                            .pUserData = nullptr};
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance.get(), "vkCreateDebugUtilsMessengerEXT"));
    auto result = VK_ERROR_EXTENSION_NOT_PRESENT;
    VkDebugUtilsMessengerEXT messenger;
    if (func)
        result = func(instance.get(), &info, nullptr, &messenger);
    vk_assert(result, "failed to setup debug messenger");
    return std::shared_ptr<debug_messenger_t>(messenger, std::default_delete<debug_messenger_t>{instance});
}

std::shared_ptr<surface_t> make_surface(std::shared_ptr<instance_t> const &instance, GLFWwindow *wnd)
{
    VkSurfaceKHR surface;
    vk_assert(glfwCreateWindowSurface(instance.get(), wnd, nullptr, &surface), "Failed to create window surface.");
    return std::shared_ptr<surface_t>(surface, std::default_delete<surface_t>(instance));
}

void device_extensions_support(VkPhysicalDevice device, const std::vector<const char *> &extensions)
{
    uint32_t count{};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    auto properties = std::vector<VkExtensionProperties>(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, properties.data());
    for (auto extension : extensions)
    {
        auto it = std::find_if(
            properties.cbegin(), properties.cend(),
            [name = extension](const VkExtensionProperties &props)
            {
                return std::strcmp(props.extensionName, name) == 0;
            });
        if (it == std::end(properties))
            RAISE(std::string{"unsupported extension: "} + extension);
    }
}

uint32_t check_device_support(VkPhysicalDevice device, const std::vector<const char *> &extensions)
{
    if (extensions.size() > 0)
    {
        device_extensions_support(device, extensions);
    }
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    uint32_t score{};
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 10;
    score += properties.limits.maxImageDimension2D;
    uint32_t shader_support = features.geometryShader ? 1 : 0;
    return score * shader_support;
}

VkBool32 check_surface_support(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t qfm_index)
{
    VkBool32 support{};
    vkGetPhysicalDeviceSurfaceSupportKHR(device, qfm_index, surface, &support);
    return support;
}

device_properties::device_properties(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    constexpr std::array<VkQueueFlags, 3> queue_flags{VK_QUEUE_GRAPHICS_BIT,
                                                      VK_QUEUE_COMPUTE_BIT,
                                                      VK_QUEUE_TRANSFER_BIT};
    uint32_t prop_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &prop_count, nullptr);
    std::vector<VkQueueFamilyProperties> qfm_properties(prop_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &prop_count, qfm_properties.data());
    std::unordered_map<VkQueueFlags, std::list<uint32_t>> map;
    for (uint32_t i{}; i < qfm_properties.size(); ++i)
    {
        for (auto flag : queue_flags)
        {
            if (qfm_properties[i].queueFlags & flag)
            {
                map[flag].push_front(i);
            }
        }
    }
    auto &graphics_qfm_indices = map[VK_QUEUE_GRAPHICS_BIT];
    if (graphics_qfm_indices.empty())
    {
        RAISE("There are no available graphics queue families.");
    }
    else
    {
        auto iter = std::find_if(graphics_qfm_indices.begin(), graphics_qfm_indices.end(),
                                 [surface, device](uint32_t index)
                                 { return check_surface_support(device, surface, index); });
        if (iter != graphics_qfm_indices.end())
        {
            present_qfm = graphics_qfm = *iter;
        }
    }
    auto &compute_qfm_indices = map[VK_QUEUE_COMPUTE_BIT];
    if (compute_qfm_indices.empty())
    {
        RAISE("There are no available compute queue families.");
    }
    else
    {
        compute_qfm = compute_qfm_indices.front();
    }
    auto &transfer_qfm_indices = map[VK_QUEUE_TRANSFER_BIT];
    if (transfer_qfm_indices.empty())
    {
        RAISE("There are no available transfer queue families.");
    }
    else
    {
        transfer_qfm = transfer_qfm_indices.front();
    }
}

auto get_physical_device(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t count{};
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count == 0)
    {
        RAISE("No physical devices are available.");
    }
    std::vector<char const *> extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());
    VkPhysicalDevice phys{};
    device_properties props;
    uint32_t highest_score{};
    for (auto device : devices)
    {
        uint32_t score = check_device_support(device, extensions);
        try
        {
            if (score > highest_score)
            {
                props = device_properties{device, surface};
                phys = device;
                highest_score = score;
            }
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Device " << device << ". " << ex.what() << '\n';
        }
    }
    if (!phys)
    {
        RAISE("Failed to find a suitable physical device.");
    }
    return std::make_tuple(phys, props);
}

std::shared_ptr<device_t> make_device(VkPhysicalDevice phys, std::set<uint32_t> const &qfmset)
{
    float priority{1};
    std::vector<VkDeviceQueueCreateInfo> queue_infos(qfmset.size());
    std::transform(std::begin(qfmset), std::end(qfmset), std::begin(queue_infos),
                   [priority](uint32_t qfm_index)
                   {
                       return VkDeviceQueueCreateInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                      .queueFamilyIndex = qfm_index,
                                                      .queueCount = 1,
                                                      .pQueuePriorities = &priority};
                   });
    VkPhysicalDeviceFeatures features{};
    char const *extension_names[]{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo info{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                            .queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
                            .pQueueCreateInfos = queue_infos.data(),
                            .enabledExtensionCount = 1,
                            .ppEnabledExtensionNames = extension_names,
                            .pEnabledFeatures = &features};
    VkDevice device;
    vk_assert(vkCreateDevice(phys, &info, nullptr, &device), "Failed to create a device.");
    return std::shared_ptr<device_t>(device, std::default_delete<device_t>{});
}

void renderer::create_core()
{
    _instance = make_instance();
#if defined DEBUG || _DEBUG
    _messenger = make_debug_messenger(_instance);
#endif
    _surface = make_surface(_instance, _wnd.get());
    std::tie(_physdevice, _props) = get_physical_device(_instance.get(), _surface.get());
    _device = make_device(_physdevice, std::set<uint32_t>{_props.graphics_qfm,
                                                          _props.present_qfm,
                                                          _props.transfer_qfm});
    vkGetDeviceQueue(_device.get(), _props.graphics_qfm, 0, &_graphics_queue);
    vkGetDeviceQueue(_device.get(), _props.present_qfm, 0, &_present_queue);
    vkGetDeviceQueue(_device.get(), _props.transfer_qfm, 0, &_transfer_queue);
}

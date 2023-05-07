#include <renderer.hpp>
#include <vector>
#include <algorithm>

bool operator==(VkSurfaceFormatKHR const &left, VkSurfaceFormatKHR const &right)
{
    return left.colorSpace == right.colorSpace && left.format == right.format;
}

bool check_swapchain_format(std::vector<VkSurfaceFormatKHR> const &formats, VkSurfaceFormatKHR const &format)
{
    return formats.end() != std::find(formats.begin(), formats.end(), format);
}

bool check_swapchain_mode(std::vector<VkPresentModeKHR> const &modes, VkPresentModeKHR const &mode)
{
    return modes.end() != std::find(modes.begin(), modes.end(), mode);
}

uint32_t minmax(uint32_t min, uint32_t value, uint32_t max)
{
    return std::max(min, std::min(value, max));
}

void get_swapchain_extent(VkSurfaceCapabilitiesKHR const &capabilities, VkExtent2D &extent)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        extent = capabilities.currentExtent;
    }
    else
    {
        extent.width = minmax(capabilities.minImageExtent.width, extent.width, capabilities.maxImageExtent.width);
        extent.height = minmax(capabilities.minImageExtent.height, extent.height, capabilities.maxImageExtent.height);
    }
}

auto make_swapchain(VkPhysicalDevice physdevice,
                    std::shared_ptr<device_t> const &device,
                    std::shared_ptr<surface_t> const &surface,
                    uint32_t graphics_qfm, uint32_t present_qfm,
                    VkExtent2D &extent, VkFormat &fmt)
{
    VkSurfaceFormatKHR format{.format = fmt, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkPresentModeKHR mode{VK_PRESENT_MODE_MAILBOX_KHR};
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physdevice, surface.get(), &capabilities);
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physdevice, surface.get(), &count, nullptr);
    if (count == 0)
    {
        RAISE("Supported swapchain formats don't exist.");
    }
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physdevice, surface.get(), &count, formats.data());
    vkGetPhysicalDeviceSurfacePresentModesKHR(physdevice, surface.get(), &count, nullptr);
    if (count == 0)
    {
        RAISE("Supported swapchain present modes don't exist.");
    }
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physdevice, surface.get(), &count, modes.data());
    if (!check_swapchain_format(formats, format))
    {
        format = formats.front();
    }
    if (!check_swapchain_mode(modes, mode))
    {
        mode = VK_PRESENT_MODE_FIFO_KHR;
    }
    get_swapchain_extent(capabilities, extent);
    // кол-во изображений не должно превосходить допустимое кол-во изображений
    count = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);
    VkSwapchainCreateInfoKHR info{.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                  .surface = surface.get(),
                                  .minImageCount = count,
                                  .imageFormat = format.format,
                                  .imageColorSpace = format.colorSpace,
                                  .imageExtent = extent,
                                  .imageArrayLayers = 1,
                                  .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                  .preTransform = capabilities.currentTransform,
                                  .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                  .presentMode = mode,
                                  .clipped = VK_TRUE,
                                  .oldSwapchain = nullptr};
    VkSwapchainKHR swapchain;
    if (graphics_qfm != present_qfm)
    {
        uint32_t qfm_array[]{graphics_qfm, present_qfm};
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = qfm_array;
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        vk_assert(vkCreateSwapchainKHR(device.get(), &info, nullptr, &swapchain), "Failed to create a swapchain in concurrent mode.");
    }
    else
    {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vk_assert(vkCreateSwapchainKHR(device.get(), &info, nullptr, &swapchain), "Failed to create a swapchain in exclusive mode.");
    }
    fmt = format.format;
    return std::shared_ptr<swapchain_t>(swapchain, std::default_delete<swapchain_t>{device});
}

auto make_pipeline_layout(std::shared_ptr<device_t> const &device,
                          VkDescriptorSetLayout *layouts, std::size_t layouts_count)
{
    VkDynamicState states[2]{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo state_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                                .dynamicStateCount = 2,
                                                .pDynamicStates = states};
    VkPipelineLayoutCreateInfo layout_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                           .setLayoutCount = static_cast<uint32_t>(layouts_count),
                                           .pSetLayouts = layouts_count == 0 ? nullptr : layouts,
                                           .pushConstantRangeCount = 0,
                                           .pPushConstantRanges = nullptr};
    VkPipelineLayout pipeline_layout;
    vk_assert(vkCreatePipelineLayout(device.get(), &layout_info, nullptr, &pipeline_layout), "Failed to create pipeline layout.");
    return std::shared_ptr<pipeline_layout_t>(pipeline_layout, std::default_delete<pipeline_layout_t>{device});
}

auto make_render_pass(std::shared_ptr<device_t> const &device, VkFormat format)
{
    VkAttachmentDescription attachment_desc{.format = format,
                                            .samples = VK_SAMPLE_COUNT_1_BIT,
                                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    VkAttachmentReference attachment_ref{.attachment = 0,
                                         .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass_desc{.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      .colorAttachmentCount = 1,
                                      .pColorAttachments = &attachment_ref};
    VkSubpassDependency dependency{.srcSubpass = VK_SUBPASS_EXTERNAL,
                                   .dstSubpass = 0,
                                   .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   .srcAccessMask = 0,
                                   .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
    VkRenderPassCreateInfo info{.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                .attachmentCount = 1,
                                .pAttachments = &attachment_desc,
                                .subpassCount = 1,
                                .pSubpasses = &subpass_desc,
                                .dependencyCount = 1,
                                .pDependencies = &dependency};
    VkRenderPass render_pass;
    vk_assert(vkCreateRenderPass(device.get(), &info, nullptr, &render_pass), "Failed to create render pass.");
    return std::shared_ptr<render_pass_t>(render_pass, std::default_delete<render_pass_t>{device});
}

auto make_image_view(std::shared_ptr<device_t> const &device, VkImage image, VkFormat format)
{
    VkComponentMapping components{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                  .a = VK_COMPONENT_SWIZZLE_IDENTITY};
    VkImageSubresourceRange range{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1};
    VkImageViewCreateInfo info{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                               .image = image,
                               .viewType = VK_IMAGE_VIEW_TYPE_2D,
                               .format = format,
                               .components = components,
                               .subresourceRange = range};
    VkImageView image_view;
    vk_assert(vkCreateImageView(device.get(), &info, nullptr, &image_view), "Failed to create image view.");
    return std::shared_ptr<image_view_t>(image_view, std::default_delete<image_view_t>{device});
}

auto make_image_views(std::shared_ptr<device_t> const &device, VkSwapchainKHR swapchain, VkFormat format)
{
    uint32_t count;
    vkGetSwapchainImagesKHR(device.get(), swapchain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(device.get(), swapchain, &count, images.data());
    std::vector<std::shared_ptr<image_view_t>> image_views(count);
    for (std::size_t i{}; i < images.size(); ++i)
    {
        image_views[i] = make_image_view(device, images[i], format);
    }
    return image_views;
}

auto make_framebuffer(std::shared_ptr<device_t> const &device, VkRenderPass render_pass, VkExtent2D extent,
                      VkImageView image_view)
{
    VkFramebufferCreateInfo info{.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                 .renderPass = render_pass,
                                 .attachmentCount = 1,
                                 .pAttachments = &image_view,
                                 .width = extent.width,
                                 .height = extent.height,
                                 .layers = 1};
    VkFramebuffer buffer;
    vk_assert(vkCreateFramebuffer(device.get(), &info, nullptr, &buffer), "Failed to create a framebuffer.");
    return std::shared_ptr<framebuffer_t>(buffer, std::default_delete<framebuffer_t>{device});
}

auto make_framebuffers(std::shared_ptr<device_t> const &device, VkRenderPass render_pass, VkExtent2D extent,
                       std::shared_ptr<image_view_t> const *image_views, std::size_t count)
{
    std::vector<std::shared_ptr<framebuffer_t>> framebuffers(count);
    for (std::size_t i{}; i < count; ++i)
    {
        framebuffers[i] = make_framebuffer(device, render_pass, extent, image_views[i].get());
    }
    return framebuffers;
}

auto make_pipeline(std::shared_ptr<device_t> const &device, VkExtent2D extent,
                   VkPipelineLayout pipeline_layout, VkRenderPass render_pass,
                   VkPipelineShaderStageCreateInfo const *stages, std::size_t stages_count)
{
    VkViewport viewport{.x = 0,
                        .y = 0,
                        .width = static_cast<float>(extent.width),
                        .height = static_cast<float>(extent.height),
                        .minDepth = 0,
                        .maxDepth = 1};
    VkRect2D scissor{.offset = {0, 0},
                     .extent = extent};
    VkPipelineViewportStateCreateInfo viewport_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                                    .viewportCount = 1,
                                                    .pViewports = &viewport,
                                                    .scissorCount = 1,
                                                    .pScissors = &scissor};
    VkPipelineRasterizationStateCreateInfo rasterization_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                              .depthClampEnable = VK_FALSE,
                                                              .rasterizerDiscardEnable = VK_FALSE,
                                                              .polygonMode = VK_POLYGON_MODE_FILL,
                                                              .cullMode = VK_CULL_MODE_BACK_BIT,
                                                              .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                              .depthBiasEnable = VK_FALSE,
                                                              .depthBiasConstantFactor = 0,
                                                              .depthBiasClamp = 0,
                                                              .depthBiasSlopeFactor = 0,
                                                              .lineWidth = 1};
    VkPipelineMultisampleStateCreateInfo multisample_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                                                          .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                                                          .sampleShadingEnable = VK_FALSE,
                                                          .minSampleShading = 1,
                                                          .pSampleMask = nullptr,
                                                          .alphaToCoverageEnable = VK_FALSE,
                                                          .alphaToOneEnable = VK_FALSE};
    VkPipelineColorBlendAttachmentState attachment_state{.blendEnable = VK_FALSE,
                                                         .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                                                         .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                                                         .colorBlendOp = VK_BLEND_OP_ADD,
                                                         .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                                                         .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                                                         .alphaBlendOp = VK_BLEND_OP_ADD,
                                                         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
    VkPipelineColorBlendStateCreateInfo colblend_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                      .logicOpEnable = VK_FALSE,
                                                      .logicOp = VK_LOGIC_OP_COPY,
                                                      .attachmentCount = 1,
                                                      .pAttachments = &attachment_state};
    VkGraphicsPipelineCreateInfo info{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                      .stageCount = static_cast<uint32_t>(stages_count),
                                      .pStages = stages,
                                      .pVertexInputState = nullptr,
                                      .pInputAssemblyState = nullptr,
                                      .pViewportState = &viewport_info,
                                      .pRasterizationState = &rasterization_info,
                                      .pMultisampleState = &multisample_info,
                                      .pDepthStencilState = nullptr,
                                      .pColorBlendState = &colblend_info,
                                      .pDynamicState = nullptr,
                                      .layout = pipeline_layout,
                                      .renderPass = render_pass,
                                      .subpass = 0,
                                      .basePipelineHandle = nullptr,
                                      .basePipelineIndex = -1};
    VkPipeline pipeline;
    vk_assert(vkCreateGraphicsPipelines(device.get(), nullptr, 1, &info, nullptr, &pipeline), "Failed to create pipeline.");
    return std::shared_ptr<pipeline_t>(pipeline, std::default_delete<pipeline_t>{device});
}

std::unique_ptr<fence_t> make_fence(std::shared_ptr<device_t> const &device);
std::unique_ptr<semaphore_t> make_semaphore(std::shared_ptr<device_t> const &device);

void resize_fences(std::shared_ptr<device_t> const &device, std::size_t new_size,
                   std::vector<std::unique_ptr<fence_t>> &fences)
{
    std::size_t old_size = fences.size();
    fences.resize(new_size);
    for (std::size_t i{old_size}; i < new_size; ++i)
    {
        fences[i] = make_fence(device);
    }
}

void resize_semaphores(std::shared_ptr<device_t> const &device, std::size_t new_size,
                       std::vector<std::unique_ptr<semaphore_t>> &semaphores)
{
    std::size_t old_size = semaphores.size();
    semaphores.resize(new_size);
    for (std::size_t i{old_size}; i < new_size; ++i)
    {
        semaphores[i] = make_semaphore(device);
    }
}
std::unique_ptr<command_pool_t> make_command_pool(std::shared_ptr<device_t> const &device, uint32_t qfm_index, VkCommandPoolCreateFlags flags);
void allocate_command_buffers(VkDevice device, VkCommandPool cmdpool, std::vector<VkCommandBuffer> &buffers);
void free_command_buffers(VkDevice device, VkCommandPool cmdpool, std::vector<VkCommandBuffer> const &buffers);

void renderer::create_pipeline()
{
    clear_framebuffers();
    VkFormat format{VK_FORMAT_B8G8R8A8_UNORM};
    _swapchain = make_swapchain(_physdevice, _device, _surface, _props.graphics_qfm, _props.present_qfm, _extent, format);
    _pipeline_layout = make_pipeline_layout(_device, _descset_layouts.data(), _descset_layouts.size());
    _render_pass = make_render_pass(_device, format);
    _image_views = make_image_views(_device, _swapchain.get(), format);
    _framebuffers = make_framebuffers(_device, _render_pass.get(), _extent, _image_views.data(), _image_views.size());
    //_pipeline = make_pipeline(_device, _extent, _pipeline_layout.get(), _render_pass.get(), _shader_stages.data(), _shader_stages.size());
    resize_fences(_device, _image_views.size(), _frame_sync_fences);
    resize_semaphores(_device, _image_views.size(), _frame_submit_semaphores);
    resize_semaphores(_device, _image_views.size(), _frame_present_semaphores);
    _frame_cmdbuffers.resize(_image_views.size());
    if (!_frame_cmdpool)
    {
        _frame_cmdpool = make_command_pool(_device, _props.graphics_qfm, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }
    allocate_command_buffers(_device.get(), _frame_cmdpool.get(), _frame_cmdbuffers);
}

void renderer::clear_framebuffers()
{
    if (_render_pass)
    {
        vk_assert(vkDeviceWaitIdle(_device.get()), "Failed to wait idles to free frame command buffers.");
        free_command_buffers(_device.get(), _frame_cmdpool.get(), _frame_cmdbuffers);
    }
}
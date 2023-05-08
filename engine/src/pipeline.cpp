#include <renderer.hpp>

void renderer::create_pipeline()
{
    if (!_shader_stages.empty())
    {
        VkDynamicState states[2]{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
        VkPipelineDynamicStateCreateInfo state_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                                    .dynamicStateCount = 2,
                                                    .pDynamicStates = states};
        VkPipelineLayoutCreateInfo layout_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                               .setLayoutCount = static_cast<uint32_t>(_set_layouts.size()),
                                               .pSetLayouts = _set_layouts.data(),
                                               .pushConstantRangeCount = 0,
                                               .pPushConstantRanges = nullptr};
        VkPipelineLayout pipeline_layout;
        vk_assert(vkCreatePipelineLayout(_device.get(), &layout_info, nullptr, &pipeline_layout), "Failed to create pipeline layout.");
        _pipeline_layout = std::unique_ptr<pipeline_layout_t>(pipeline_layout, std::default_delete<pipeline_layout_t>{_device});

        VkViewport viewport{.x = 0,
                            .y = 0,
                            .width = static_cast<float>(_extent.width),
                            .height = static_cast<float>(_extent.height),
                            .minDepth = 0,
                            .maxDepth = 1};
        VkRect2D scissor{.offset = {0, 0},
                         .extent = _extent};
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
                                          .stageCount = static_cast<uint32_t>(_shader_stages.size()),
                                          .pStages = _shader_stages.data(),
                                          .pVertexInputState = nullptr,
                                          .pInputAssemblyState = nullptr,
                                          .pViewportState = &viewport_info,
                                          .pRasterizationState = &rasterization_info,
                                          .pMultisampleState = &multisample_info,
                                          .pDepthStencilState = nullptr,
                                          .pColorBlendState = &colblend_info,
                                          .pDynamicState = nullptr,
                                          .layout = _pipeline_layout.get(),
                                          .renderPass = _render_pass.get(),
                                          .subpass = 0,
                                          .basePipelineHandle = nullptr,
                                          .basePipelineIndex = -1};
        VkPipeline pipeline;
        vk_assert(vkCreateGraphicsPipelines(_device.get(), nullptr, 1, &info, nullptr, &pipeline), "Failed to create pipeline.");
        _pipeline = std::unique_ptr<pipeline_t>(pipeline, std::default_delete<pipeline_t>{_device});
    }
}
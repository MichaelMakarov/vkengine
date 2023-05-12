#include <renderer.hpp>
#include <fstream>

std::vector<char> read_shader_file(std::string const &filename)
{
    std::ifstream fin{filename, std::ios::ate | std::ios::binary};
    if (!fin.is_open())
    {
        RAISE("Failed to open file " + filename);
    }
    std::size_t size = static_cast<std::size_t>(fin.tellg());
    std::vector<char> buffer(size);
    fin.seekg(0);
    fin.read(buffer.data(), size);
    fin.close();
    return buffer;
}

auto make_shader_module(std::shared_ptr<device_t> const &device, std::vector<char> const &buffer)
{
    VkShaderModuleCreateInfo info{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                  .codeSize = buffer.size(),
                                  .pCode = reinterpret_cast<uint32_t const *>(buffer.data())};
    VkShaderModule shader_module;
    vk_assert(vkCreateShaderModule(device.get(), &info, nullptr, &shader_module), "Failed to create shader module.");
    return std::unique_ptr<shader_module_t>(shader_module, std::default_delete<shader_module_t>{device});
}

auto make_shader_stage(VkShaderStageFlagBits stage, VkShaderModule shader_module)
{
    return VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                           .stage = stage,
                                           .module = shader_module,
                                           .pName = "main"};
}

auto make_vertex_input_state()
{
    return VkPipelineVertexInputStateCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                                                .vertexBindingDescriptionCount = 0,
                                                .vertexAttributeDescriptionCount = 0};
}

auto make_input_assembly_state(VkPrimitiveTopology topology)
{
    return VkPipelineInputAssemblyStateCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                                                  .topology = topology,
                                                  .primitiveRestartEnable = VK_FALSE};
}

auto make_pipeline_layout(std::shared_ptr<device_t> const &device)
{
    VkPipelineLayoutCreateInfo layout_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                           .setLayoutCount = 0,
                                           .pSetLayouts = nullptr,
                                           .pushConstantRangeCount = 0,
                                           .pPushConstantRanges = nullptr};
    VkPipelineLayout pipeline_layout;
    vk_assert(vkCreatePipelineLayout(device.get(), &layout_info, nullptr, &pipeline_layout), "Failed to create pipeline layout.");
    return std::unique_ptr<pipeline_layout_t>(pipeline_layout, std::default_delete<pipeline_layout_t>{device});
}

struct vertex_fragment_drawer : public pipeline_drawer
{
    std::vector<char> vert_code, frag_code;
    std::unique_ptr<shader_module_t> vert_module, frag_module;
    std::unique_ptr<pipeline_layout_t> pipeline_layout;
    std::unique_ptr<pipeline_t> pipeline;

    void draw_command(VkCommandBuffer cmdbuffer) override
    {
        vkCmdBindPipeline(cmdbuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get());
        vkCmdDraw(cmdbuffer, 3, 1, 0, 0);
    }
    void set_pipeline(std::unique_ptr<pipeline_t> &&new_pipeline) override
    {
        pipeline = std::move(new_pipeline);
    }
};

void renderer::add_shader(std::string const &vs_filename, std::string fs_filename)
{
    auto drawer = std::make_unique<vertex_fragment_drawer>();
    drawer->vert_code = read_shader_file(vs_filename);
    drawer->frag_code = read_shader_file(fs_filename);
    drawer->vert_module = make_shader_module(_device, drawer->vert_code);
    drawer->frag_module = make_shader_module(_device, drawer->frag_code);
    drawer->pipeline_layout = make_pipeline_layout(_device);
    pipeline_builder builder;
    builder.shader_stages.resize(2);
    builder.shader_stages[0] = make_shader_stage(VK_SHADER_STAGE_VERTEX_BIT, drawer->vert_module.get());
    builder.shader_stages[1] = make_shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, drawer->frag_module.get());
    builder.pipeline_layout = drawer->pipeline_layout.get();
    builder.rasterization_state.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
    builder.input_assembly_state.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    _pipeline_drawers.push_back(std::move(drawer));
    _pipeline_builders.push_back(std::move(builder));
}

void renderer::create_pipeline()
{
    for (std::size_t i{}; i < _pipeline_builders.size(); ++i)
    {
        auto &builder = _pipeline_builders[i];
        builder.viewport.width = static_cast<float>(_extent.width);
        builder.viewport.height = static_cast<float>(_extent.height);
        builder.scissor.extent = _extent;
        builder.render_pass = _render_pass.get();
        _pipeline_drawers[i]->set_pipeline(builder.create_pipeline(_device));
    }
}

pipeline_builder::pipeline_builder()
    : viewport{
          .x = 0,
          .y = 0,
          .minDepth = 0,
          .maxDepth = 1,
      },
      scissor{
          .offset = {0, 0},
      },
      rasterization_state{
          .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
          .depthClampEnable = VK_FALSE,
          .rasterizerDiscardEnable = VK_FALSE,
          .cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT,
          .frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE,
          .depthBiasEnable = VK_FALSE,
          .depthBiasConstantFactor = 0,
          .depthBiasClamp = 0,
          .depthBiasSlopeFactor = 0,
          .lineWidth = 1,
      },
      multisample_state{
          .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
          .rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
          .sampleShadingEnable = VK_FALSE,
          .minSampleShading = 1,
          .pSampleMask = nullptr,
          .alphaToCoverageEnable = VK_FALSE,
          .alphaToOneEnable = VK_FALSE,
      },
      attachment_state{
          .blendEnable = VK_FALSE,
          .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
          .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
          .colorBlendOp = VK_BLEND_OP_ADD,
          .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
          .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
          .alphaBlendOp = VK_BLEND_OP_ADD,
          .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      },
      vertex_input_state{
          .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      },
      input_assembly_state{
          .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
          .primitiveRestartEnable = VK_FALSE,
      },
      pipeline_layout{}, render_pass{}
{
}

pipeline_builder::pipeline_builder(pipeline_builder &&other) noexcept
    : viewport{other.viewport},
      scissor{other.scissor},
      rasterization_state{other.rasterization_state},
      multisample_state{other.multisample_state},
      attachment_state{other.attachment_state},
      vertex_input_state{other.vertex_input_state},
      input_assembly_state{other.input_assembly_state},
      pipeline_layout{other.pipeline_layout},
      render_pass{other.render_pass},
      shader_stages{std::move(other.shader_stages)}
{
}

pipeline_builder &pipeline_builder::operator=(pipeline_builder &&other) noexcept
{
    viewport = other.viewport;
    scissor = other.scissor;
    rasterization_state = other.rasterization_state;
    multisample_state = other.multisample_state;
    attachment_state = other.attachment_state;
    vertex_input_state = other.vertex_input_state;
    input_assembly_state = other.input_assembly_state;
    pipeline_layout = other.pipeline_layout;
    render_pass = other.render_pass;
    shader_stages = std::move(other.shader_stages);
    return *this;
}

std::unique_ptr<pipeline_t> pipeline_builder::create_pipeline(std::shared_ptr<device_t> const &device) const
{
    VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    VkPipelineColorBlendStateCreateInfo color_blend_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &attachment_state,
    };
    VkGraphicsPipelineCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = nullptr,
        .layout = pipeline_layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };
    VkPipeline pipeline;
    vk_assert(vkCreateGraphicsPipelines(device.get(), nullptr, 1, &info, nullptr, &pipeline), "Failed to create pipeline.");
    return std::unique_ptr<pipeline_t>(pipeline, std::default_delete<pipeline_t>{device});
}
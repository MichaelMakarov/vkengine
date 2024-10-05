#pragma once

#include "graphics_types.hpp"

#include <optional>

class PipelineBuilder {
    VkPipelineVertexInputStateCreateInfo vertex_input_state_{default_vertex_input_state()};
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_{default_input_assembly_state()};
    std::optional<VkPipelineTessellationStateCreateInfo> tesselation_state_;
    VkPipelineViewportStateCreateInfo viewport_state_{default_viewport_state()};
    VkPipelineRasterizationStateCreateInfo rasterization_state_{default_rasterization_state()};
    VkPipelineMultisampleStateCreateInfo multisample_state_{default_multisample_state()};
    std::optional<VkPipelineDepthStencilStateCreateInfo> depth_stencil_state_;
    VkPipelineColorBlendStateCreateInfo color_blend_state_{default_color_blend_state()};
    std::optional<VkPipelineDynamicStateCreateInfo> dynamic_state_;
    VkGraphicsPipelineCreateInfo pipeline_info_{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pVertexInputState = &vertex_input_state_,
        .pInputAssemblyState = &input_assembly_state_,
        .pViewportState = &viewport_state_,
        .pRasterizationState = &rasterization_state_,
        .pMultisampleState = &multisample_state_,
        .pColorBlendState = &color_blend_state_,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };

  public:
    void set_shader_stages(VkPipelineShaderStageCreateInfo const *shader_stages, size_t stages_count) {
        pipeline_info_.stageCount = static_cast<uint32_t>(stages_count);
        pipeline_info_.pStages = shader_stages;
    }

    void set_vertex_input_state(VkPipelineVertexInputStateCreateInfo const &info) {
        vertex_input_state_ = info;
    }

    void set_input_assembly_state(VkPipelineInputAssemblyStateCreateInfo const &info) {
        input_assembly_state_ = info;
    }

    void set_tesselation_state(VkPipelineTessellationStateCreateInfo const &info) {
        tesselation_state_ = info;
        pipeline_info_.pTessellationState = &tesselation_state_.value();
    }

    void set_viewport_state(VkPipelineViewportStateCreateInfo const &info) {
        viewport_state_ = info;
    }

    void set_rasterization_state(VkPipelineRasterizationStateCreateInfo const &info) {
        rasterization_state_ = info;
    }

    void set_multisample_state(VkPipelineMultisampleStateCreateInfo const &info) {
        multisample_state_ = info;
    }

    void set_depth_stencil_state(VkPipelineDepthStencilStateCreateInfo const &info) {
        depth_stencil_state_ = info;
        pipeline_info_.pDepthStencilState = &depth_stencil_state_.value();
    }

    void set_color_blend_state(VkPipelineColorBlendStateCreateInfo const &info) {
        color_blend_state_ = info;
    }

    void set_dynamic_state(VkPipelineDynamicStateCreateInfo const &info) {
        dynamic_state_ = info;
        pipeline_info_.pDynamicState = &dynamic_state_.value();
    }

    void set_pipeline_layout(VkPipelineLayout pipeline_layout) {
        pipeline_info_.layout = pipeline_layout;
    }

    void set_render_pass(VkRenderPass render_pass) {
        pipeline_info_.renderPass = render_pass;
    }

    unique_ptr_of<VkPipeline> make_pipeline(shared_ptr_of<VkDevice> device);

    static VkPipelineVertexInputStateCreateInfo default_vertex_input_state();

    static VkPipelineInputAssemblyStateCreateInfo default_input_assembly_state();

    static VkPipelineViewportStateCreateInfo default_viewport_state();

    static VkPipelineRasterizationStateCreateInfo default_rasterization_state();

    static VkPipelineMultisampleStateCreateInfo default_multisample_state();

    static VkPipelineColorBlendStateCreateInfo default_color_blend_state();

    static VkPipelineColorBlendAttachmentState default_color_blend_attachment();
};
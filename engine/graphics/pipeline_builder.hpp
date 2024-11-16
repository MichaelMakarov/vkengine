#pragma once

#include "graphics/graphics_types.hpp"

#include <optional>

class VertexInputStateProvider {
    VkPipelineVertexInputStateCreateInfo vertex_input_state_{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };
    std::vector<VkVertexInputBindingDescription> vertex_bindings_;
    std::vector<VkVertexInputAttributeDescription> vertex_attributes_;

  public:
    VkPipelineVertexInputStateCreateInfo const &get() const {
        return vertex_input_state_;
    }

    void set_vertex_bindings(std::vector<VkVertexInputBindingDescription> &&vertex_bindings) {
        vertex_bindings_.swap(vertex_bindings);
        vertex_input_state_.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_bindings_.size());
        vertex_input_state_.pVertexBindingDescriptions = vertex_bindings_.data();
    }

    void set_vertex_attributes(std::vector<VkVertexInputAttributeDescription> &&vertex_attributes) {
        vertex_attributes_.swap(vertex_attributes);
        vertex_input_state_.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes_.size());
        vertex_input_state_.pVertexAttributeDescriptions = vertex_attributes_.data();
    }
};

class ViewportStateProvider {
    VkPipelineViewportStateCreateInfo viewport_state_{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    };
    std::vector<VkViewport> viewports_;
    std::vector<VkRect2D> scissors_;

  public:
    VkPipelineViewportStateCreateInfo const &get() const {
        return viewport_state_;
    }

    void set_viewports(std::vector<VkViewport> &&viewports) {
        viewports_.swap(viewports);
        viewport_state_.viewportCount = static_cast<uint32_t>(viewports_.size());
        viewport_state_.pViewports = viewports_.data();
    }

    void set_scissors(std::vector<VkRect2D> &&scissors) {
        scissors_.swap(scissors);
        viewport_state_.scissorCount = static_cast<uint32_t>(scissors_.size());
        viewport_state_.pScissors = scissors_.data();
    }
};

class ColorBlendStateProvider {
    std::vector<VkPipelineColorBlendAttachmentState> attachments_;
    VkPipelineColorBlendStateCreateInfo color_blend_state_;

  public:
    ColorBlendStateProvider(VkBool32 logic_op_enable, VkLogicOp logic_op)
        : attachments_{VkPipelineColorBlendAttachmentState{
              .blendEnable = VK_FALSE,
              .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
              .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
              .colorBlendOp = VK_BLEND_OP_ADD,
              .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
              .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
              .alphaBlendOp = VK_BLEND_OP_ADD,
              .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
          }}
        , color_blend_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
              .logicOpEnable = logic_op_enable,
              .logicOp = logic_op,
              .attachmentCount = static_cast<uint32_t>(attachments_.size()),
              .pAttachments = attachments_.data(),
          } {
    }

    ColorBlendStateProvider()
        : ColorBlendStateProvider(VK_FALSE, VK_LOGIC_OP_COPY) {
    }

    VkPipelineColorBlendStateCreateInfo const &get() const {
        return color_blend_state_;
    }

    void set_color_blend_attachments(std::vector<VkPipelineColorBlendAttachmentState> &&attachments) {
        attachments_.swap(attachments);
        color_blend_state_.attachmentCount = static_cast<uint32_t>(attachments_.size());
        color_blend_state_.pAttachments = attachments_.data();
    }
};

class PipelineBuilder {
    shared_ptr_of<VkDevice> device_;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VertexInputStateProvider vertex_input_state_;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_;
    std::optional<VkPipelineTessellationStateCreateInfo> tesselation_state_;
    ViewportStateProvider viewport_state_;
    VkPipelineRasterizationStateCreateInfo rasterization_state_;
    VkPipelineMultisampleStateCreateInfo multisample_state_;
    std::optional<VkPipelineDepthStencilStateCreateInfo> depth_stencil_state_;
    ColorBlendStateProvider color_blend_state_;
    std::optional<VkPipelineDynamicStateCreateInfo> dynamic_state_;
    shared_ptr_of<VkPipelineLayout> pipeline_layout_;
    VkGraphicsPipelineCreateInfo pipeline_info_;

  public:
    explicit PipelineBuilder(shared_ptr_of<VkDevice> device);

    unique_ptr_of<VkPipeline> make_pipeline();

    void set_shader_stages(std::vector<VkPipelineShaderStageCreateInfo> &&shader_stages) {
        shader_stages_ = std::move(shader_stages);
        pipeline_info_.stageCount = static_cast<uint32_t>(shader_stages_.size());
        pipeline_info_.pStages = shader_stages_.data();
    }

    void set_vertex_input_state(VertexInputStateProvider &&provider) {
        vertex_input_state_ = std::move(provider);
        pipeline_info_.pVertexInputState = &vertex_input_state_.get();
    }

    void set_input_assembly_state(VkPipelineInputAssemblyStateCreateInfo const &info) {
        input_assembly_state_ = info;
    }

    void set_tesselation_state(VkPipelineTessellationStateCreateInfo const &info) {
        tesselation_state_ = info;
        pipeline_info_.pTessellationState = &tesselation_state_.value();
    }

    void set_viewport_state(ViewportStateProvider &&provider) {
        viewport_state_ = std::move(provider);
        pipeline_info_.pViewportState = &viewport_state_.get();
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

    void set_color_blend_state(ColorBlendStateProvider &&provider) {
        color_blend_state_ = std::move(provider);
        pipeline_info_.pColorBlendState = &color_blend_state_.get();
    }

    void set_dynamic_state(VkPipelineDynamicStateCreateInfo const &info) {
        dynamic_state_ = info;
        pipeline_info_.pDynamicState = &dynamic_state_.value();
    }

    void set_pipeline_layout(shared_ptr_of<VkPipelineLayout> pipeline_layout) {
        pipeline_layout_.swap(pipeline_layout);
        pipeline_info_.layout = pipeline_layout_.get();
    }

    void set_render_pass(VkRenderPass render_pass) {
        pipeline_info_.renderPass = render_pass;
    }
};
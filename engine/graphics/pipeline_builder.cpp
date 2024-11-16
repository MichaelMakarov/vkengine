#include "pipeline_builder.hpp"

#include "graphics/graphics_manager.hpp"

namespace {

    VkPipelineInputAssemblyStateCreateInfo default_input_assembly_state() {
        return VkPipelineInputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };
    }

    VkPipelineRasterizationStateCreateInfo default_rasterization_state() {
        return VkPipelineRasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0,
            .depthBiasClamp = 0,
            .depthBiasSlopeFactor = 0,
            .lineWidth = 1,
        };
    }

    VkPipelineMultisampleStateCreateInfo default_multisample_state() {
        return VkPipelineMultisampleStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
        };
    }

} // namespace

PipelineBuilder::PipelineBuilder(shared_ptr_of<VkDevice> device)
    : device_{device}
    , input_assembly_state_(default_input_assembly_state())
    , rasterization_state_{default_rasterization_state()}
    , multisample_state_{default_multisample_state()}
    , pipeline_info_{
          .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
          .pVertexInputState = &vertex_input_state_.get(),
          .pInputAssemblyState = &input_assembly_state_,
          .pViewportState = &viewport_state_.get(),
          .pRasterizationState = &rasterization_state_,
          .pMultisampleState = &multisample_state_,
          .pColorBlendState = &color_blend_state_.get(),
          .subpass = 0,
          .basePipelineHandle = nullptr,
          .basePipelineIndex = -1,
      } {
}

unique_ptr_of<VkPipeline> PipelineBuilder::make_pipeline() {
    return GraphicsManager::make_pipeline(device_, pipeline_info_);
}


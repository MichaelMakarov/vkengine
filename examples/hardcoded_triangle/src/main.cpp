#include "graphics/graphics_renderer.hpp"
#include "graphics/graphics_manager.hpp"
#include "utility/logger_provider.hpp"

#include <vector>

namespace {

    class triangle_pipeline {
        shared_ptr_of<VkDevice> device_;
        unique_ptr_of<VkShaderModule> vertex_shader_module_;
        unique_ptr_of<VkShaderModule> fragment_shader_module_;
        unique_ptr_of<VkPipelineLayout> pipeline_layout_;
        unique_ptr_of<VkPipeline> pipeline_;
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
        VkPipelineVertexInputStateCreateInfo input_state_;
        VkPipelineInputAssemblyStateCreateInfo assembly_state_;
        VkViewport viewport_;
        VkRect2D scissor_;
        VkPipelineViewportStateCreateInfo viewport_state_;
        VkPipelineRasterizationStateCreateInfo rasterization_state_;
        VkPipelineMultisampleStateCreateInfo multisample_state_;
        VkPipelineColorBlendAttachmentState attachment_state_;
        VkPipelineColorBlendStateCreateInfo color_blend_state_;
        VkGraphicsPipelineCreateInfo pipeline_info_;

      public:
        triangle_pipeline(shared_ptr_of<VkDevice> device, std::string_view vertex_shader, std::string_view fragment_shader)
            : device_{device}
            , vertex_shader_module_{graphics_manager::make_shader_module(device, vertex_shader)}
            , fragment_shader_module_{graphics_manager::make_shader_module(device, fragment_shader)}
            , pipeline_layout_{graphics_manager::make_pipeline_layout(device, {}, {})}
            , shader_stages_(2) {
            shader_stages_[0] = graphics_manager::make_shader_stage(vertex_shader_module_.get(), VK_SHADER_STAGE_VERTEX_BIT);
            shader_stages_[1] = graphics_manager::make_shader_stage(fragment_shader_module_.get(), VK_SHADER_STAGE_FRAGMENT_BIT);
            input_state_ = VkPipelineVertexInputStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 0,
                .pVertexBindingDescriptions = nullptr,
                .vertexAttributeDescriptionCount = 0,
                .pVertexAttributeDescriptions = nullptr,
            };
            assembly_state_ = VkPipelineInputAssemblyStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE,
            };
            viewport_ = VkViewport{
                .x = 0,
                .y = 0,
                .minDepth = 0,
                .maxDepth = 1,
            };
            scissor_ = VkRect2D{
                .offset = {.x = 0, .y = 0},
                .extent = {.width = 0, .height = 0},
            };
            viewport_state_ = VkPipelineViewportStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .pViewports = &viewport_,
                .scissorCount = 1,
                .pScissors = &scissor_,
            };
            rasterization_state_ = VkPipelineRasterizationStateCreateInfo{
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
            };
            multisample_state_ = VkPipelineMultisampleStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE,
            };
            attachment_state_ = VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };
            color_blend_state_ = VkPipelineColorBlendStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &attachment_state_,
            };
            pipeline_info_ = VkGraphicsPipelineCreateInfo{
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = static_cast<uint32_t>(shader_stages_.size()),
                .pStages = shader_stages_.data(),
                .pVertexInputState = &input_state_,
                .pInputAssemblyState = &assembly_state_,
                .pViewportState = &viewport_state_,
                .pRasterizationState = &rasterization_state_,
                .pMultisampleState = &multisample_state_,
                .pDepthStencilState = nullptr,
                .pColorBlendState = &color_blend_state_,
                .pDynamicState = nullptr,
                .layout = pipeline_layout_.get(),
                .renderPass = nullptr,
                .subpass = 0,
                .basePipelineHandle = nullptr,
                .basePipelineIndex = -1,
            };
        }

        void update_pipeline(rendering_info const &info) {
            viewport_.width = static_cast<float>(info.extent.width);
            viewport_.height = static_cast<float>(info.extent.height);
            scissor_.extent = info.extent;
            pipeline_info_.renderPass = info.render_pass;
            pipeline_ = graphics_manager::make_pipeline(device_, pipeline_info_);
        }

        void draw(VkCommandBuffer command_buffer) {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());
            vkCmdDraw(command_buffer, 3, 1, 0, 0);
        }
    };

    class triangle_provider : public pipeline_provider {
        triangle_pipeline colorful_triangle_;
        triangle_pipeline monochrome_triangle_;
        triangle_pipeline *current_triangle_{nullptr};
        std::function<void()> callback_;

      public:
        triangle_provider(shared_ptr_of<VkDevice> device)
            : colorful_triangle_{device, "colorful_triangle.vert.spv", "colorful_triangle.frag.spv"}
            , monochrome_triangle_{device, "monochrome_triangle.vert.spv", "monochrome_triangle.frag.spv"}
            , current_triangle_{&monochrome_triangle_} {
        }

        void update_command_buffer(VkCommandBuffer command_buffer) override {
            current_triangle_->draw(command_buffer);
        }

        void set_update_rendering(std::function<void()> const &callback) override {
            callback_ = callback;
        }

        void update_rendering(rendering_info const &info) {
            colorful_triangle_.update_pipeline(info);
            monochrome_triangle_.update_pipeline(info);
        }

        void change_pipeline() {
            if (current_triangle_ == &monochrome_triangle_) {
                current_triangle_ = &colorful_triangle_;
            } else {
                current_triangle_ = &monochrome_triangle_;
            }
            if (callback_) {
                callback_();
            }
        }
    };

} // namespace

int main() {
    try {
        graphics_renderer renderer{window_info{
            .title = "Hardcoded triangle application",
            .width = 600,
            .height = 600,
        }};
        auto device = renderer.get_device_context().get_device();
        auto provider = std::make_shared<triangle_provider>(device);
        renderer.set_pipeline_provider(provider);
        renderer.set_keyboard_callback([provider](key_value value, key_action action, key_modifier modifier) {
            if (value == key_value::key_space && action == key_action::release) {
                provider->change_pipeline();
            }
        });
        renderer.set_extent_changed_callback([provider](rendering_info const &info) { provider->update_rendering(info); });

        renderer.run();

    } catch (const std::exception &ex) {
        logger_provider::println(ex.what());
        return 1;
    }
    return 0;
}
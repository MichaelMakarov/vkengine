#include "graphics/graphics_manager.hpp"
#include "graphics/graphics_renderer.hpp"
#include "graphics/pipeline_builder.hpp"
#include "utility/logger_provider.hpp"

#include <vector>

namespace {

    class triangle_pipeline {
        shared_ptr_of<VkDevice> device_;
        unique_ptr_of<VkShaderModule> vertex_shader_module_;
        unique_ptr_of<VkShaderModule> fragment_shader_module_;
        unique_ptr_of<VkPipelineLayout> pipeline_layout_;
        unique_ptr_of<VkPipeline> pipeline_;
        PipelineBuilder builder_;
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
        VkViewport viewport_{
            .x = 0,
            .y = 0,
            .minDepth = 0,
            .maxDepth = 1,
        };
        VkRect2D scissor_{
            .offset = {.x = 0, .y = 0},
            .extent = {.width = 0, .height = 0},
        };
        VkPipelineColorBlendAttachmentState attachment_{PipelineBuilder::default_color_blend_attachment()};

      public:
        triangle_pipeline(shared_ptr_of<VkDevice> device, std::string_view vertex_shader, std::string_view fragment_shader)
            : device_{device}
            , vertex_shader_module_{GraphicsManager::make_shader_module(device, vertex_shader)}
            , fragment_shader_module_{GraphicsManager::make_shader_module(device, fragment_shader)}
            , pipeline_layout_{GraphicsManager::make_pipeline_layout(device, {}, {})}
            , shader_stages_(2) {
            shader_stages_[0] = GraphicsManager::make_shader_stage(vertex_shader_module_.get(), VK_SHADER_STAGE_VERTEX_BIT);
            shader_stages_[1] = GraphicsManager::make_shader_stage(fragment_shader_module_.get(), VK_SHADER_STAGE_FRAGMENT_BIT);
            builder_.set_shader_stages(shader_stages_.data(), shader_stages_.size());
            builder_.set_pipeline_layout(pipeline_layout_.get());
            builder_.set_viewport_state(VkPipelineViewportStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .pViewports = &viewport_,
                .scissorCount = 1,
                .pScissors = &scissor_,
            });
            builder_.set_color_blend_state(VkPipelineColorBlendStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &attachment_,
            });
        }

        void update_pipeline(rendering_info const &info) {
            viewport_.width = static_cast<float>(info.extent.width);
            viewport_.height = static_cast<float>(info.extent.height);
            scissor_.extent = info.extent;
            builder_.set_render_pass(info.render_pass);
            pipeline_ = builder_.make_pipeline(device_);
        }

        void draw(VkCommandBuffer command_buffer) {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());
            vkCmdDraw(command_buffer, 3, 1, 0, 0);
        }
    };

    class triangle_provider : public PipelineProvider {
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
        GraphicsRenderer renderer{WindowConfig{
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
        renderer.set_rendering_changed_callback([provider](rendering_info const &info) { provider->update_rendering(info); });
        renderer.run();

    } catch (const std::exception &ex) {
        LoggerProvider::println(ex.what());
        return 1;
    }
    return 0;
}
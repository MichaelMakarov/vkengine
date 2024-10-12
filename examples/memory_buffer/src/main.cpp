#include "matrix.hpp"
#include "plain_mesh.hpp"

#include "geometry/vertex.hpp"
#include "graphics/buffer_manager.hpp"
#include "graphics/graphics_manager.hpp"
#include "graphics/graphics_renderer.hpp"
#include "graphics/memory_buffer.hpp"
#include "graphics/pipeline_builder.hpp"
#include "utility/logger_provider.hpp"

#include <array>

namespace {

    class PipelineHandler {
        PlainMesh mesh_;
        Matrix matrix_;
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
        VkVertexInputBindingDescription binding_description_{Vertex2d::get_binding_description(0)};
        std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions_{Vertex2d::get_attribute_description(0)};

      public:
        PipelineHandler(shared_ptr_of<VkDevice> device,
                        VkPhysicalDevice phys_device,
                        uint32_t transfer_qfm,
                        std::string_view vertex_shader,
                        std::string_view fragment_shader)
            : mesh_{device, phys_device, transfer_qfm}
            , matrix_{device, phys_device}
            , device_{device}
            , vertex_shader_module_{GraphicsManager::make_shader_module(device, vertex_shader)}
            , fragment_shader_module_{GraphicsManager::make_shader_module(device, fragment_shader)}
            , pipeline_layout_{GraphicsManager::make_pipeline_layout(device, {matrix_.get_layout()}, {})}
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
                .logicOp = VkLogicOp::VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &attachment_,
            });
            builder_.set_vertex_input_state(VkPipelineVertexInputStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &binding_description_,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions_.size()),
                .pVertexAttributeDescriptions = attribute_descriptions_.data(),
            });
        }

        void update_pipeline(GraphicsRenderer::Context const &info) {
            matrix_.setup_buffers(info.images_count, info.surface_extent);
            viewport_.width = static_cast<float>(info.surface_extent.width);
            viewport_.height = static_cast<float>(info.surface_extent.height);
            scissor_.extent = info.surface_extent;
            builder_.set_render_pass(info.render_pass);
            pipeline_ = builder_.make_pipeline(device_);
        }

        void update_command_buffer(VkCommandBuffer command_buffer, size_t index) {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());
            matrix_.setup_command_buffer(pipeline_layout_.get(), command_buffer, index);
            mesh_.draw(command_buffer);
        }

        void update_frame(size_t index) {
            matrix_.update_content(index);
        }
    };

} // namespace

int main() {
    using namespace std::placeholders;
    try {
        GraphicsRenderer renderer{WindowConfig{
            .title = "Load mesh from memory",
            .width = 600,
            .height = 500,
        }};
        auto const &device_ctx = renderer.get_device_context();
        PipelineHandler handler(device_ctx.get_device(),
                                device_ctx.get_physical_device(),
                                device_ctx.get_transfer_qfm(),
                                "shader.vert.spv",
                                "shader.frag.spv");
        renderer.set_context_changed_callback(std::bind(&PipelineHandler::update_pipeline, &handler, _1));
        renderer.set_update_command_callback(std::bind(&PipelineHandler::update_command_buffer, &handler, _1, _2));
        renderer.set_update_frame_callback(std::bind(&PipelineHandler::update_frame, &handler, _1));
        renderer.run();
    } catch (const std::exception &ex) {
        LoggerProvider::println(ex.what());
    }

    return 0;
}
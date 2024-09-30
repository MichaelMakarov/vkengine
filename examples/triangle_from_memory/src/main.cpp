#include "geometry/vertex.hpp"
#include "graphics/graphics_manager.hpp"
#include "graphics/graphics_renderer.hpp"
#include "graphics/memory_buffer.hpp"
#include "graphics/pipeline_builder.hpp"
#include "utility/logger_provider.hpp"

#include <array>

namespace {

    class data_provider {
        const std::array<vertex2d, 3> vertices_;

      public:
        data_provider()
            : vertices_{
                  vertex2d(glm::vec2(0.0f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f)),
                  vertex2d(glm::vec2(-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f)),
                  vertex2d(glm::vec2(0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f)),
              } {
        }

        memory_buffer::config get_buffer_config() const {
            return memory_buffer::config{
                .size = sizeof(vertices_),
                .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                .mode = VK_SHARING_MODE_EXCLUSIVE,
                .properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            };
        }

        uint32_t get_vertex_count() const {
            return static_cast<uint32_t>(vertices_.size());
        }

        size_t get_data_size() const {
            return sizeof(vertices_);
        }

        void const *get_data_ptr() const {
            return vertices_.data();
        }
    };

    class pipeline_handler : public pipeline_provider {
        data_provider data_provider_;
        shared_ptr_of<VkDevice> device_;
        unique_ptr_of<VkShaderModule> vertex_shader_module_;
        unique_ptr_of<VkShaderModule> fragment_shader_module_;
        unique_ptr_of<VkPipelineLayout> pipeline_layout_;
        unique_ptr_of<VkPipeline> pipeline_;
        memory_buffer vertex_memory_buffer_;
        pipeline_builder builder_;
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
        VkPipelineColorBlendAttachmentState attachment_{pipeline_builder::default_color_blend_attachment()};
        VkVertexInputBindingDescription binding_description_{vertex2d::get_binding_description(0)};
        std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions_{vertex2d::get_attribute_description(0)};

      public:
        pipeline_handler(shared_ptr_of<VkDevice> device,
                         VkPhysicalDevice phys_device,
                         std::string_view vertex_shader,
                         std::string_view fragment_shader)
            : device_{device}
            , vertex_shader_module_{graphics_manager::make_shader_module(device, vertex_shader)}
            , fragment_shader_module_{graphics_manager::make_shader_module(device, fragment_shader)}
            , pipeline_layout_{graphics_manager::make_pipeline_layout(device, {}, {})}
            , vertex_memory_buffer_{device, phys_device, data_provider_.get_buffer_config()}
            , shader_stages_(2) {
            shader_stages_[0] = graphics_manager::make_shader_stage(vertex_shader_module_.get(), VK_SHADER_STAGE_VERTEX_BIT);
            shader_stages_[1] = graphics_manager::make_shader_stage(fragment_shader_module_.get(), VK_SHADER_STAGE_FRAGMENT_BIT);
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
            vertex_memory_buffer_.fill(data_provider_.get_data_ptr(), data_provider_.get_data_size());
        }

        void update_pipeline(rendering_info const &info) {
            viewport_.width = static_cast<float>(info.extent.width);
            viewport_.height = static_cast<float>(info.extent.height);
            scissor_.extent = info.extent;
            builder_.set_render_pass(info.render_pass);
            pipeline_ = builder_.make_pipeline(device_);
        }

        void update_command_buffer(VkCommandBuffer command_buffer) override {
            std::array<VkBuffer, 1> buffers{vertex_memory_buffer_.get_buffer()};
            std::array<VkDeviceSize, 1> offsets{0};
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());
            vkCmdBindVertexBuffers(command_buffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
            vkCmdDraw(command_buffer, data_provider_.get_vertex_count(), 1, 0, 0);
        }

        void set_update_rendering(std::function<void()> const &callback) {
            // nothing to do
        }
    };

} // namespace

int main() {
    try {
        graphics_renderer renderer{window_info{
            .title = "Triangle loaded from memory",
            .width = 600,
            .height = 500,
        }};
        auto const &device_ctx = renderer.get_device_context();
        auto provider = std::make_shared<pipeline_handler>(device_ctx.get_device(),
                                                           device_ctx.get_physical_device(),
                                                           "triangle.vert.spv",
                                                           "triangle.frag.spv");
        renderer.set_pipeline_provider(provider);
        renderer.set_rendering_changed_callback([provider](rendering_info const &info) { provider->update_pipeline(info); });
        renderer.run();
    } catch (const std::exception &ex) {
        logger_provider::println(ex.what());
    }

    return 0;
}
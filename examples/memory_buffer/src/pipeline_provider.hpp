#pragma once

#include "matrix_descriptor.hpp"
#include "plain_mesh.hpp"
#include "texture_descriptor.hpp"

#include "graphics/allocator_interface.hpp"
#include "graphics/commander.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/shader_context.hpp"
#include "graphics/graphics_renderer.hpp"
#include "graphics/descriptor_set.hpp"

class PipelineProvider {
    std::shared_ptr<AllocatorInterface> allocator_;
    Commander transfer_;
    Commander barrier_;
    PlainMesh mesh_;
    std::shared_ptr<TextureDescriptor> texture_;
    std::shared_ptr<MatrixDescriptor> matrix_;
    DescriptorSet descriptor_set_;
    PipelineBuilder builder_;
    ShaderContext vertex_shader_;
    ShaderContext fragment_shader_;
    shared_ptr_of<VkPipelineLayout> pipeline_layout_;
    unique_ptr_of<VkPipeline> pipeline_;

  public:
    PipelineProvider(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device, uint32_t transfer_qfm, uint32_t graphics_qfm);

    void update_command_buffer(VkCommandBuffer command_buffer, size_t image_index);

    void setup_pipeline(GraphicsRenderer::Context const &info);

    void update_image(size_t image_index);
};
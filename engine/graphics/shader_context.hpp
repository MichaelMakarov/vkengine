#pragma once

#include "graphics_types.hpp"

#include <string_view>

class ShaderContext {
    unique_ptr_of<VkShaderModule> shader_module_;
    VkShaderStageFlagBits stage_;

  public:
    ShaderContext(shared_ptr_of<VkDevice> device, std::string_view filename, VkShaderStageFlagBits stage);

    VkPipelineShaderStageCreateInfo get_shader_stage() const;
};
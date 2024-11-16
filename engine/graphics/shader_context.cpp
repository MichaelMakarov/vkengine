#include "shader_context.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

#include <fstream>

namespace {

    std::vector<uint8_t> read_shader(std::string_view filename) {
        std::ifstream fin{filename.data(), std::ios::ate | std::ios::binary};
        if (!fin.is_open()) {
            raise_error("Failed to open shader file {}", filename);
        }
        std::vector<uint8_t> buffer(static_cast<size_t>(fin.tellg()));
        fin.seekg(0);
        fin.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        fin.close();
        return buffer;
    }

} // namespace

ShaderContext::ShaderContext(shared_ptr_of<VkDevice> device, std::string_view filename, VkShaderStageFlagBits stage)
    : stage_{stage} {
    auto buffer = read_shader(filename);
    VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = buffer.size(),
        .pCode = reinterpret_cast<uint32_t const *>(buffer.data()),
    };
    VkShaderModule shader_module;
    vk_assert(vkCreateShaderModule(device.get(), &info, nullptr, &shader_module), "Failed to create a shader module.");
    shader_module_ = unique_ptr_of<VkShaderModule>(shader_module, [device](VkShaderModule shader_module) {
        debug_println("delete shader module");
        vkDestroyShaderModule(device.get(), shader_module, nullptr);
    });
}

VkPipelineShaderStageCreateInfo ShaderContext::get_shader_stage() const {
    return VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage_,
        .module = shader_module_.get(),
        .pName = "main",
    };
}

#include <renderer.hpp>
#include <fstream>

std::vector<char> read_shader_file(std::string const &filename)
{
    std::ifstream fin{filename, std::ios::ate | std::ios::binary};
    if (!fin.is_open())
    {
        RAISE("Failed to open file " + filename);
    }
    std::size_t size = static_cast<std::size_t>(fin.tellg());
    std::vector<char> buffer(size);
    fin.seekg(0);
    fin.read(buffer.data(), size);
    fin.close();
    return buffer;
}

auto make_shader_module(std::shared_ptr<device_t> const &device, std::vector<char> const &buffer)
{
    VkShaderModuleCreateInfo info{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                  .codeSize = buffer.size(),
                                  .pCode = reinterpret_cast<uint32_t const *>(buffer.data())};
    VkShaderModule shader_module;
    vk_assert(vkCreateShaderModule(device.get(), &info, nullptr, &shader_module), "Failed to create shader module.");
    return std::unique_ptr<shader_module_t>(shader_module, std::default_delete<shader_module_t>{device});
}

auto make_shader_stage(VkShaderStageFlagBits stage, VkShaderModule shader_module)
{
    return VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                           .stage = stage,
                                           .module = shader_module,
                                           .pName = "main"};
}

VkShaderStageFlagBits get_shader_stage_flag(shader_type type)
{
    VkShaderStageFlagBits flag;
    switch (type)
    {
    case shader_type::vertex:
        flag = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case shader_type::fragment:
        flag = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    default:
        RAISE("Unsupported shader type.");
    }
    return flag;
}

void renderer::add_shader(shader_info const &info)
{
    VkShaderStageFlagBits flag = get_shader_stage_flag(info.type);
    auto buffer = read_shader_file(info.filename);
    auto shader_module = make_shader_module(_device, buffer);
    _shader_stages.push_back(make_shader_stage(flag, shader_module.get()));
    _shader_modules.push_back(std::move(shader_module));
}
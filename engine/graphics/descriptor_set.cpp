#include "descriptor_set.hpp"

#include "graphics_manager.hpp"

#include <algorithm>

using namespace std::placeholders;

DescriptorSet::DescriptorSet(shared_ptr_of<VkDevice> device, std::vector<std::shared_ptr<DescriptorInterface>> const &descriptors)
    : device_{device} {
    std::vector<VkDescriptorSetLayoutBinding> bindings(descriptors.size());
    std::transform(descriptors.begin(), descriptors.end(), bindings.begin(), std::bind(&DescriptorInterface::get_binding, _1));
    set_layout_ = GraphicsManager::make_descriptor_set_layout(device_, bindings);
    descriptors_.resize(descriptors.size());
    for (size_t i = 0; i < descriptors.size(); ++i) {
        descriptors_[i].descriptor = descriptors[i];
        descriptors_[i].type = bindings[i].descriptorType;
    }
}

void DescriptorSet::set_swapchain_images_count(uint32_t images_count) {
    if (images_count > descriptor_sets_.size()) {
        std::vector<VkDescriptorPoolSize> pool_sizes(descriptors_.size());
        std::transform(descriptors_.begin(), descriptors_.end(), pool_sizes.begin(), [images_count](auto info) {
            return VkDescriptorPoolSize{.type = info.type, .descriptorCount = images_count};
        });
        descriptor_pool_ = GraphicsManager::make_descriptor_pool(device_, images_count, pool_sizes);
        std::vector<VkDescriptorSetLayout> set_layouts(images_count, set_layout_.get());
        descriptor_sets_ = GraphicsManager::allocate_descriptor_sets(device_, descriptor_pool_.get(), set_layouts);
        std::vector<VkWriteDescriptorSet> write_sets(descriptors_.size() * descriptor_sets_.size());
        for (size_t i = 0, n = 0; i < descriptor_sets_.size(); ++i) {
            for (size_t j = 0; j < descriptors_.size(); ++j, ++n) {
                write_sets[n] = descriptors_[j].descriptor->get_write(descriptor_sets_[i], i);
            }
        }
        vkUpdateDescriptorSets(device_.get(), static_cast<uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
    }
}

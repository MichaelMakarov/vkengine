#pragma once

#include "command_interface.hpp"

class ImageCopyCommand : public CommandInterface {
    VkBuffer buffer_;
    VkImage image_;
    uint32_t width_;
    uint32_t height_;

  public:
    ImageCopyCommand(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void execute(VkCommandBuffer command_buffer) override;
};
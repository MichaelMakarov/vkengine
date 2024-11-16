#pragma once

#include "command_interface.hpp"

#include <deque>

class Commander {
    shared_ptr_of<VkDevice> device_;
    shared_ptr_of<VkCommandPool> command_pool_;
    VkQueue queue_;
    std::deque<std::unique_ptr<CommandInterface>> commands_;

    unique_ptr_of<VkCommandBuffer> begin_command();
    void end_command(VkCommandBuffer command_buffer);

  public:
    Commander(shared_ptr_of<VkDevice> device, uint32_t qfm_index, uint32_t queue_index);

    void add_command(std::unique_ptr<CommandInterface> command);

    void execute();
};
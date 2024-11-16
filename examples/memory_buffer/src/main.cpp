#include "pipeline_provider.hpp"

#include "utility/log.hpp"

int main() {
    using namespace std::placeholders;
    try {
        GraphicsRenderer renderer{WindowConfig{
            .title = "Load mesh from memory",
            .width = 600,
            .height = 500,
        }};
        auto const &device = renderer.get_device_context();
        PipelineProvider provider(device.get_device(), device.get_physical_device(), device.get_transfer_qfm(), device.get_graphics_qfm());
        renderer.set_context_changed_callback(std::bind(&PipelineProvider::setup_pipeline, &provider, _1));
        renderer.set_update_command_callback(std::bind(&PipelineProvider::update_command_buffer, &provider, _1, _2));
        renderer.set_update_frame_callback(std::bind(&PipelineProvider::update_image, &provider, _1));
        renderer.run();
    } catch (const std::exception &ex) {
        error_println("{}", ex.what());
    }
    return 0;
}
#include "pipeline_provider.hpp"

#include "graphics/graphics_manager.hpp"
#include "graphics/graphics_renderer.hpp"
#include "utility/log.hpp"

int main() {
    using namespace std::placeholders;
    try {
        GraphicsRenderer renderer{WindowConfig{
            .title = "Hardcoded triangle application",
            .width = 600,
            .height = 600,
        }};
        auto device = renderer.get_device_context().get_device();
        PipelineProvider provider(device);
        renderer.set_keyboard_callback([&provider, &renderer](key_value value, key_action action, key_modifier modifier) {
            if (value == key_value::key_space && action == key_action::release) {
                provider.change_pipeline();
                renderer.update_render_pass();
            }
        });
        renderer.set_context_changed_callback(std::bind(&PipelineProvider::setup_pipeline, &provider, _1));
        renderer.set_update_command_callback(std::bind(&PipelineProvider::update_command_buffer, &provider, _1, _2));
        renderer.run();

    } catch (const std::exception &ex) {
        error_println("{}", ex.what());
        return 1;
    }
    return 0;
}
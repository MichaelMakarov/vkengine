#include "graphics/graphics_renderer.hpp"
#include "utility/logger_provider.hpp"
#include "geometry/vertex.hpp"

namespace {

    const vertex2d vertices[3]{
        vertex2d{
            .point = {0.0f, -0.5f},
            .color = {1.0f, 0.0f, 0.0f},
        },
        vertex2d{
            .point = {0.5f, 0.5f},
            .color = {0.0f, 1.0f, 0.0f},
        },
        vertex2d{
            .point = {-0.5f, 0.5f},
            .color = {0.0f, 0.0f, 1.0f},
        },
    };

} // namespace

int main() {
    try {
        graphics_renderer renderer{window_info{
            .title = "Triangle loaded from memory",
            .width = 600,
            .height = 500,
        }};
        renderer.run();
    } catch (const std::exception &ex) {
        logger_provider::println(ex.what());
    }

    return 0;
}
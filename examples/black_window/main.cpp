#include "graphics/graphics_renderer.hpp"

#include <iostream>

int main() {
    try {
        GraphicsRenderer{WindowConfig{
                        .title = "Colored window application",
                        .width = 600,
                        .height = 500,
                    }}
            .run();
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
    return 0;
}
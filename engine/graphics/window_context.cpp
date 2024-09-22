#include "window_context.hpp"

#include "graphics_error.hpp"

namespace {

    class glfwinstance {
      public:
        glfwinstance() {
            if (!glfwInit()) {
                raise_error("Failed to initialize glfw library");
            }
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        ~glfwinstance() {
            glfwTerminate();
        }
    };

    glfwinstance instance;

    std::unordered_map<GLFWwindow *, window_context> map;

    void resize_callback(GLFWwindow *window, int width, int height) {
        if (auto window_ctx = window_context::get_window_context(window)) {
            window_ctx->execute_resize_callback(width, height);
        }
    }

    void keyboard_callback(GLFWwindow *window, int key, int /* scancode */, int action, int mods) {
        if (auto window_ctx = window_context::get_window_context(window)) {
            window_ctx->execute_keyboard_callback(static_cast<key_value>(key), static_cast<key_action>(action), static_cast<key_modifier>(mods));
        }
    }

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
        if (auto window_ctx = window_context::get_window_context(window)) {
            window_ctx->execute_mouse_button_callback(static_cast<mouse_button>(button),
                                               static_cast<key_action>(action),
                                               static_cast<key_modifier>(mods));
        }
    }

    void mouse_scroll_callback(GLFWwindow *window, double /* xoffset */, double yoffset) {
        if (auto window_ctx = window_context::get_window_context(window)) {
            window_ctx->execute_mouse_scroll_callback(yoffset);
        }
    }

    void cursor_callback(GLFWwindow *window, double x, double y) {
        if (auto window_ctx = window_context::get_window_context(window)) {
            window_ctx->execute_cursor_callback(x, y);
        }
    }

    void delete_window(GLFWwindow *window) {
        map.erase(window);
        glfwDestroyWindow(window);
    }

} // namespace

unique_ptr_of<GLFWwindow *> window_context::make_window(window_info const &info) {
    auto window = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
    if (window == nullptr) {
        raise_error("Failed to create a GLFW window.")
    }
    glfwSetWindowSizeCallback(window, &resize_callback);
    glfwSetKeyCallback(window, &keyboard_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetScrollCallback(window, &mouse_scroll_callback);
    glfwSetCursorPosCallback(window, &cursor_callback);

    if (!map.emplace(window, window_context()).second) {
        raise_error("Failed to map a new created window_manager.");
    }

    return unique_ptr_of<GLFWwindow *>(window, &delete_window);
}

window_context *window_context::window_context::get_window_context(GLFWwindow *window) {
    auto iter = map.find(window);
    if (iter == map.end()) {
        return nullptr;
    }
    return &iter->second;
}

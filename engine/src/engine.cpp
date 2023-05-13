#include <renderer.hpp>
#include <observer.hpp>
#include <GLFW/glfw3.h>
#include <functional>

void std::default_delete<GLFWwindow>::operator()(GLFWwindow *wnd) const
{
    glfwDestroyWindow(wnd);
}

namespace
{
    struct glfwinstance
    {
        glfwinstance()
        {
            if (!glfwInit())
            {
                RAISE("Failed to initialize glfw library");
            }
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        ~glfwinstance()
        {
            glfwTerminate();
        }
    };
    glfwinstance __intstance__;

    std::function<void(int, int)> resize_func;
    std::shared_ptr<keyboard_observer> keyboard;
    std::shared_ptr<mouse_observer> mouse;
    std::shared_ptr<cursor_observer> cursor;

    void resize_callback(GLFWwindow *, int w, int h)
    {
        if (resize_func)
            resize_func(w, h);
    }

    void keyboard_callback(GLFWwindow *, int key, int /* scancode */, int action, int mods)
    {
        if (keyboard)
            keyboard->key_callback(static_cast<key_value>(key),
                                   static_cast<key_action>(action),
                                   static_cast<key_modifier>(mods));
    }

    void mouse_button_callback(GLFWwindow *, int button, int action, int mods)
    {
        if (mouse)
            mouse->button_callback(static_cast<mouse_button>(button),
                                   static_cast<key_action>(action),
                                   static_cast<key_modifier>(mods));
    }

    void mouse_scroll_callback(GLFWwindow *, double /* xoffset */, double yoffset)
    {
        if (mouse)
            mouse->scroll_callback(yoffset);
    }

    void cursor_callback(GLFWwindow *, double x, double y)
    {
        if (cursor)
            cursor->position_callback(x, y);
    }

    GLFWwindow *create_window(int w, int h, char const *title)
    {
        auto wnd = glfwCreateWindow(w, h, title, nullptr, nullptr);
        if (!wnd)
        {
            RAISE("Failed to create window.");
        }
        glfwSetWindowSizeCallback(wnd, &resize_callback);
        glfwSetKeyCallback(wnd, &keyboard_callback);
        glfwSetMouseButtonCallback(wnd, &mouse_button_callback);
        glfwSetScrollCallback(wnd, &mouse_scroll_callback);
        glfwSetCursorPosCallback(wnd, &cursor_callback);
        return wnd;
    }
}

std::shared_ptr<engine> engine::make_engine(unsigned w, unsigned h, char const *title)
{
    static std::shared_ptr<engine> _engine{new engine(w, h, title)};
    return _engine;
}

engine::engine(unsigned w, unsigned h, char const *title)
{
    _wnd.reset(create_window(w, h, title), std::default_delete<GLFWwindow>{});
    _renderer.reset(new graphics_renderer{_wnd});
}

engine::~engine()
{
}

void engine::run()
{
    using namespace std::placeholders;
    resize_func = std::bind(&engine::on_window_resized, this, _1, _2);
    _renderer->run();
    resize_func = nullptr;
}

void engine::set_observer(std::shared_ptr<keyboard_observer> const &observer)
{
    keyboard = observer;
}

void engine::set_observer(std::shared_ptr<mouse_observer> const &observer)
{
    mouse = observer;
}

void engine::set_observer(std::shared_ptr<cursor_observer> const &observer)
{
    cursor = observer;
}

void engine::on_window_resized(int w, int h)
{
    _renderer->on_window_resized(w, h);
}

void engine::add_content(std::shared_ptr<content_provider> const &provider)
{
    _renderer->add_content(provider);
}

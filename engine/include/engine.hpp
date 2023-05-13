#pragma once
#include <memory>

struct GLFWwindow;
class graphics_renderer;
class keyboard_observer;
class mouse_observer;
class cursor_observer;
class content_provider;

class engine
{
    std::shared_ptr<GLFWwindow> _wnd;
    std::unique_ptr<graphics_renderer> _renderer;
    std::shared_ptr<keyboard_observer> _keyboard;
    std::shared_ptr<mouse_observer> _mouse;
    std::shared_ptr<cursor_observer> _cursor;

    engine(unsigned w, unsigned h, char const *title);
    void on_window_resized(int, int);

public:
    ~engine();
    void run();
    void set_observer(std::shared_ptr<keyboard_observer> const &observer);
    void set_observer(std::shared_ptr<mouse_observer> const &observer);
    void set_observer(std::shared_ptr<cursor_observer> const &observer);
    void add_content(std::shared_ptr<content_provider> const &provider);

    static std::shared_ptr<engine> make_engine(unsigned w, unsigned h, char const *title);
};
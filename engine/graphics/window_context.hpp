#pragma once

#include "graphics_types.hpp"
#include "window_config.hpp"

#include <GLFW/glfw3.h>

class WindowContext {
    WindowConfig::resize_t resize_callback_;
    WindowConfig::cursor_t cursor_callback_;
    WindowConfig::keyboard_t keyboard_callback_;
    WindowConfig::mouse_button_t mouse_button_callback_;
    WindowConfig::mouse_scroll_t mouse_scroll_callback_;

  public:
    void set_resize_callback(WindowConfig::resize_t const &callback) {
        resize_callback_ = callback;
    }

    void set_cursor_callback(WindowConfig::cursor_t const &callback) {
        cursor_callback_ = callback;
    }

    void set_keyboard_callback(WindowConfig::keyboard_t const &callback) {
        keyboard_callback_ = callback;
    }

    void set_mouse_button_callback(WindowConfig::mouse_button_t const &callback) {
        mouse_button_callback_ = callback;
    }

    void set_mouse_scroll_callback(WindowConfig::mouse_scroll_t const &callback) {
        mouse_scroll_callback_ = callback;
    }

    void execute_resize_callback(int width, int height) {
        if (resize_callback_) {
            resize_callback_(width, height);
        }
    }

    void execute_cursor_callback(double x, double y) {
        if (cursor_callback_) {
            cursor_callback_(x, y);
        }
    }

    void execute_keyboard_callback(key_value value, key_action action, key_modifier modifier) {
        if (keyboard_callback_) {
            keyboard_callback_(value, action, modifier);
        }
    }

    void execute_mouse_button_callback(mouse_button button, key_action action, key_modifier modifier) {
        if (mouse_button_callback_) {
            mouse_button_callback_(button, action, modifier);
        }
    }

    void execute_mouse_scroll_callback(double offset) {
        if (mouse_scroll_callback_) {
            mouse_scroll_callback_(offset);
        }
    }

    static unique_ptr_of<GLFWwindow *> make_window(WindowConfig const &info);

    static WindowContext *get_window_context(GLFWwindow *window);
};

#pragma once

enum struct key_value
{
    key_space = 32,
    key_0 = 48,
    key_1,
    key_2,
    key_3,
    key_4,
    key_5,
    key_6,
    key_7,
    key_8,
    key_9,
    key_a = 65,
    key_b,
    key_c,
    key_d,
    key_e,
    key_f,
    key_g,
    key_h,
    key_i,
    key_j,
    key_k,
    key_l,
    key_m,
    key_n,
    key_o,
    key_p,
    key_q,
    key_r,
    key_s,
    key_t,
    key_u,
    key_v,
    key_w,
    key_x,
    key_y,
    key_z,
    key_escape = 256,
    key_enter,
    key_tab,
    key_backspace,
    key_delete = 261,
    key_right,
    key_left,
    key_down,
    key_up,
    key_caps_lock = 280,
    key_f1 = 290,
    key_f2,
    key_f3,
    key_f4,
    key_f5,
    key_f6,
    key_f7,
    key_f8,
    key_f9,
    key_f10,
    key_f11,
    key_f12,
    key_left_shift = 340,
    key_left_control,
    key_left_alt,
    key_right_shift = 344,
    key_right_control,
    key_right_alt
};

enum struct key_action
{
    release = 0,
    press,
    repeat
};

enum struct key_modifier
{
    shift = 0x0001,
    control = 0x0002,
    alt = 0x0004,
    caps_lock = 0x00010,
};

enum struct mouse_button
{
    left = 0,
    right,
    middle,
};

class keyboard_observer
{
public:
    virtual ~keyboard_observer() = default;
    virtual void key_callback(key_value, key_action, key_modifier) const = 0;
};

class mouse_observer
{
public:
    virtual ~mouse_observer() = default;
    virtual void button_callback(mouse_button, key_action, key_modifier) const = 0;
    virtual void scroll_callback(double offset) const = 0;
};

class cursor_observer
{
public:
    virtual ~cursor_observer() = default;
    virtual void position_callback(double x, double y) const = 0;
};
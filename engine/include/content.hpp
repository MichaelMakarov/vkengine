#pragma once
#include <string>
#include <signal.hpp>

enum struct polygon_mode
{
    fill,
    line,
    point
};

enum struct primitive_topology
{
    triangle_list,
    point_list,
    line_list
};

struct vertex_shader_info
{
    std::string filename;
    polygon_mode mode;
    primitive_topology topology;
};

struct fragment_shader_info
{
    std::string filename;
};

class shaders_provider
{
public:
    virtual ~shaders_provider() = default;
    virtual vertex_shader_info const *get_vertex_shader_info() const = 0;
    virtual fragment_shader_info const *get_fragment_shader_info() const = 0;
};

class content_provider
{
public:
    virtual ~content_provider() = default;
    virtual std::vector<shaders_provider const *> get_shaders_providers() const = 0;
    signal_function<void(shaders_provider const *)> shaders_provider_changed;
};
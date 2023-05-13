#include <iostream>
#include <engine.hpp>
#include <content.hpp>
#include <observer.hpp>

class monochrome_triangle_provider : public shaders_provider
{
    vertex_shader_info _vert_info{
        .filename = "monochrome_triangle.vert.spv",
        .mode = polygon_mode::fill,
        .topology = primitive_topology::triangle_list,
    };
    fragment_shader_info _frag_info{
        .filename = "monochrome_triangle.frag.spv",
    };

public:
    vertex_shader_info const *get_vertex_shader_info() const override
    {
        return &_vert_info;
    }
    fragment_shader_info const *get_fragment_shader_info() const override
    {
        return &_frag_info;
    }
};

class colorful_triangle_provider : public shaders_provider
{
    vertex_shader_info _vert_info{
        .filename = "colorful_triangle.vert.spv",
        .mode = polygon_mode::fill,
        .topology = primitive_topology::triangle_list,
    };
    fragment_shader_info _frag_info{
        .filename = "colorful_triangle.frag.spv",
    };

public:
    vertex_shader_info const *get_vertex_shader_info() const override
    {
        return &_vert_info;
    }
    fragment_shader_info const *get_fragment_shader_info() const override
    {
        return &_frag_info;
    }
};

class triangle_content_provider : public content_provider
{
    monochrome_triangle_provider _monochrome_shaders;
    colorful_triangle_provider _colorful_shaders;
    std::size_t _index{};

public:
    std::vector<shaders_provider const *> get_shaders_providers() const override
    {
        return std::vector<shaders_provider const *>{&_monochrome_shaders, &_colorful_shaders};
    }
    void change_shaders()
    {
        if (_index == 0)
            shaders_provider_changed(&_colorful_shaders);
        else
            shaders_provider_changed(&_monochrome_shaders);
        _index = (_index + 1) % 2;
    }
};

class shaders_switcher : public keyboard_observer
{
    std::shared_ptr<triangle_content_provider> _provider;

public:
    shaders_switcher(std::shared_ptr<triangle_content_provider> const &provider) : _provider{provider}
    {
    }
    void key_callback(key_value value, key_action action, key_modifier modifier) const override
    {
        if (value == key_value::key_space)
        {
            if (action == key_action::release)
            {
                _provider->change_shaders();
            }
        }
    }
};

int main()
{
    try
    {
        auto e = engine::make_engine(600, 600, "Hardcoded triangle application");
        auto provider = std::make_shared<triangle_content_provider>();
        auto switcher = std::make_shared<shaders_switcher>(provider);
        e->add_content(provider);
        e->set_observer(switcher);
        e->run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }
    return 0;
}
#pragma once
#include <memory>
#include <string>

enum struct shader_type
{
    vertex,
    fragment
};

struct shader_info
{
    std::string filename;
    shader_type type;
};

class engine
{
    class renderer *_renderer;

    engine(unsigned w, unsigned h, char const *title);

public:
    ~engine();
    void run();
    void add_shader(shader_info const &info);

    static std::shared_ptr<engine> make_engine(unsigned w, unsigned h, char const *title);
};
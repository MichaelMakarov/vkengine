#include <renderer.hpp>

std::shared_ptr<engine> engine::make_engine(unsigned w, unsigned h, char const *title)
{
    static std::shared_ptr<engine> _engine{new engine(w, h, title)};
    return _engine;
}

engine::engine(unsigned w, unsigned h, char const *title) : _renderer{new renderer(w, h, title)}
{
}

engine::~engine()
{
    delete _renderer;
}

void engine::run()
{
    _renderer->run();
}

void engine::add_shader(shader_info const &info)
{
}

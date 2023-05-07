#include <engine.hpp>
#include <renderer.hpp>

std::shared_ptr<engine> const &engine::get_engine()
{
    static std::shared_ptr<engine> _engine{new engine};
    return _engine;
}

engine::engine() : _renderer{new renderer}
{
}

engine::~engine()
{
    delete _renderer;
}

void engine::run(unsigned w, unsigned h, char const *title)
{
    _renderer->run(w, h, title);
}
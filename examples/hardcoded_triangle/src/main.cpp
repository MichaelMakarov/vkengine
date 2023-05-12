#include <iostream>
#include <engine.hpp>

int main()
{
    try
    {
        auto e = engine::make_engine(600, 600, "Hardcoded triangle application");
        e->add_shader("triangle.vert.spv", "triangle.frag.spv");
        e->run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }
    return 0;
}
#include <iostream>
#include <engine.hpp>

int main()
{

    try
    {
        auto e = engine::get_engine();
        e->run(600, 600, "Application 1 window");
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
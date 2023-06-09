#include <iostream>
#include <engine.hpp>

int main()
{

    try
    {
        engine::make_engine(600, 600, "Colored window application")->run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
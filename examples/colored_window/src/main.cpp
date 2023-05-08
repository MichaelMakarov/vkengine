#include <iostream>
#include <engine.hpp>

int main()
{

    try
    {
        engine::make_engine(600, 600, "Application 1 window")->run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
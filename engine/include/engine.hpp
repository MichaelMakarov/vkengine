#pragma once
#include <memory>

class engine
{
    class renderer *_renderer;

    engine();

public:
    ~engine();
    void run(unsigned w, unsigned h, char const *title);

    static std::shared_ptr<engine> const &get_engine();
};
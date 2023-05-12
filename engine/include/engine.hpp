#pragma once
#include <memory>
#include <string>



class engine
{
    class renderer *_renderer;

    engine(unsigned w, unsigned h, char const *title);

public:
    ~engine();
    void run();
    void add_shader(std::string const &vsfilename, std::string fsfilename);

    static std::shared_ptr<engine> make_engine(unsigned w, unsigned h, char const *title);
};
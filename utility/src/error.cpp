#include <error.hpp>
#include <stdexcept>
#include <sstream>

void raise_error(std::string const &msg, char const *file, int line)
{
    std::stringstream sstr;
    sstr << "An error occured in file " << file << " in line " << line;
    sstr << ". " << msg;
    throw std::runtime_error(sstr.str());
}
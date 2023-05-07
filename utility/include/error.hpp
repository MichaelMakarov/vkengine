#pragma once
#include <string>

void raise_error(std::string const &msg, char const *file, int line);

#define RAISE(msg) raise_error(msg, __FILE__, __LINE__);
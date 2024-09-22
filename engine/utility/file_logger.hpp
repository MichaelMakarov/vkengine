#pragma once

#include "logger_interface.hpp"

#include <fstream>

class file_logger : public logger_interface {
    std::ofstream fout_;

  public:
    file_logger(std::string_view filename);

    void print(std::string_view str) override;
};
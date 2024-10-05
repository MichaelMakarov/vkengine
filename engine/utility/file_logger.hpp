#pragma once

#include "logger_interface.hpp"

#include <fstream>

class FileLogger : public LoggerInterface {
    std::ofstream fout_;

  public:
    FileLogger(std::string_view filename);

    void print(std::string_view str) override;
};
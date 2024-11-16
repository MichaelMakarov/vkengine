#pragma once

#include "debug.hpp"
#include "log_level.hpp"

#include <string_view>

class LoggerProvider {
  public:
    static void println(LogLevel level, std::string_view str);

    static void set_log_level(LogLevel level);

    static void make_console_logger();

    static void make_file_logger(std::string_view filename);
};


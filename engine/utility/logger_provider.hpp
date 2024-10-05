#pragma once

#include <string_view>
#include <format>

class LoggerProvider {
  public:
    static void println(std::string_view str);

    static void make_console_logger();

    static void make_file_logger(std::string_view filename);
};

#if defined DEBUG || _DEBUG
#define DEBUG_APP
#endif

#if defined DEBUG_APP
#define debug_println(fmt, ...) LoggerProvider::println(std::format(fmt, __VA_ARGS__))
#else
#define debug_println(fmt, ...)
#endif
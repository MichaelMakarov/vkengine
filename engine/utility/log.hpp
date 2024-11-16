#pragma once

#include "logger_provider.hpp"

#include <format>

#if defined DEBUG_APP
#define debug_println(fmt, ...) LoggerProvider::println(LogLevel::debug, std::format(fmt, __VA_ARGS__))
#define info_println(fmt, ...) LoggerProvider::println(LogLevel::info, std::format(fmt, __VA_ARGS__))
#define error_println(fmt, ...) LoggerProvider::println(LogLevel::error, std::format(fmt, __VA_ARGS__))
#else
#define debug_println(fmt, ...)
#define info_println(fmt, ...)
#define error_println(fmt, ...)
#endif
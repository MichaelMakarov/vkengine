#pragma once

#include <format>
#include <stdexcept>

#define raise_error(msg, ...) throw std::runtime_error(std::format("{}:{}: ", __FILE__, __LINE__) + std::format(msg, __VA_ARGS__));
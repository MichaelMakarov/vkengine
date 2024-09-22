#pragma once

#include <string_view>

class logger_interface {
  public:
    virtual ~logger_interface() = default;

    virtual void print(std::string_view str) = 0;
};
#pragma once

#include <string_view>

class LoggerInterface {
  public:
    virtual ~LoggerInterface() = default;

    virtual void print(std::string_view str) = 0;
};
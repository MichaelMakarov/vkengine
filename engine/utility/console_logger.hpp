#pragma once

#include "logger_interface.hpp"

#include <iostream>

class console_logger : public logger_interface {
  public:
    void print(std::string_view str) override {
        std::clog << str;
    }
};
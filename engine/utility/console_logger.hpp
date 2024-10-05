#pragma once

#include "logger_interface.hpp"

#include <iostream>

class ConsoleLogger : public LoggerInterface {
  public:
    void print(std::string_view str) override {
        std::clog << str;
    }
};
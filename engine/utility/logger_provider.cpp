#include "logger_provider.hpp"

#include "console_logger.hpp"
#include "file_logger.hpp"

#include <memory>

namespace {

    std::unique_ptr<LoggerInterface> global_logger = std::make_unique<ConsoleLogger>();

}

void LoggerProvider::println(std::string_view str) {
    global_logger->print(str);
    global_logger->print("\n");
}

void LoggerProvider::make_console_logger() {
    global_logger = std::make_unique<ConsoleLogger>();
}

void LoggerProvider::make_file_logger(std::string_view filename) {
    global_logger = std::make_unique<FileLogger>(filename);
}

#include "logger_provider.hpp"

#include "console_logger.hpp"
#include "file_logger.hpp"

#include <memory>

namespace {

    std::unique_ptr<logger_interface> global_logger = std::make_unique<console_logger>();

}

void logger_provider::println(std::string_view str) {
    global_logger->print(str);
    global_logger->print("\n");
}

void logger_provider::make_console_logger() {
    global_logger = std::make_unique<console_logger>();
}

void logger_provider::make_file_logger(std::string_view filename) {
    global_logger = std::make_unique<file_logger>(filename);
}

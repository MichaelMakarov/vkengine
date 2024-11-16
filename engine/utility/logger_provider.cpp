#include "logger_provider.hpp"

#include "console_logger.hpp"
#include "file_logger.hpp"

#include <memory>

namespace {

    struct LoggerContext {
        std::unique_ptr<LoggerInterface> logger = std::make_unique<ConsoleLogger>();
        LogLevel level = LogLevel::info;
    };

    LoggerContext logger_context;

    char const *log_level_to_str(LogLevel level) {
        switch (level) {
        case LogLevel::debug:
            return "(debug) ";
        case LogLevel::info:
            return "(info) ";
        case LogLevel::error:
            return "(error) ";
        default:
            throw std::invalid_argument("Unknown log level");
            return "(unknown) ";
        }
    }

} // namespace

void LoggerProvider::println(LogLevel level, std::string_view str) {
    if (level >= logger_context.level) {
        logger_context.logger->print(log_level_to_str(level));
        logger_context.logger->print(str);
        logger_context.logger->print("\n");
    }
}

void LoggerProvider::set_log_level(LogLevel level) {
    logger_context.level = level;
}

void LoggerProvider::make_console_logger() {
    logger_context.logger = std::make_unique<ConsoleLogger>();
}

void LoggerProvider::make_file_logger(std::string_view filename) {
    logger_context.logger = std::make_unique<FileLogger>(filename);
}

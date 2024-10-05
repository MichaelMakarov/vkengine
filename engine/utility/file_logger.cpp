#include "file_logger.hpp"

#include "error.hpp"

FileLogger::FileLogger(std::string_view filename)
{
    fout_.open(filename.data());
    if (!fout_.is_open()) {
        raise_error("Failed to open file {} to write", filename);
    }
}

void FileLogger::print(std::string_view str) {
    fout_ << str;
}

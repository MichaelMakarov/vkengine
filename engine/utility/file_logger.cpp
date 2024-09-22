#include "file_logger.hpp"

#include "error.hpp"

file_logger::file_logger(std::string_view filename)
{
    fout_.open(filename.data());
    if (!fout_.is_open()) {
        raise_error("Failed to open file {} to write", filename);
    }
}

void file_logger::print(std::string_view str) {
    fout_ << str;
}

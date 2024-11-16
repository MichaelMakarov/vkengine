#pragma once

#include <string_view>

class ImageWrapper {
    void *pixels_;
    int width_;
    int height_;
    int channels_;

  public:
    explicit ImageWrapper(std::string_view filename);

    ~ImageWrapper();

    void const *get_data() const {
        return pixels_;
    }

    size_t get_size() const {
        return static_cast<size_t>(width_ * height_ * channels_);
    }

    unsigned get_width() const {
        return static_cast<unsigned>(width_);
    }

    unsigned get_height() const {
        return static_cast<unsigned>(height_);
    }
};
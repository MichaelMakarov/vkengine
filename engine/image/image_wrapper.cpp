#include "image_wrapper.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "utility/error.hpp"
#include "utility/log.hpp"

ImageWrapper::ImageWrapper(std::string_view filename) {
    pixels_ = stbi_load(filename.data(), &width_, &height_, &channels_, STBI_rgb_alpha);
    if (pixels_ == nullptr) {
        raise_error("Failed to load image from {}.", filename);
    }
    info_println("Image {} has been read: width={}, height={}, channels={}.", filename, width_, height_, channels_);
}

ImageWrapper::~ImageWrapper() {
    stbi_image_free(pixels_);
}

#pragma once

#include "utility/debug.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>

template <typename T>
using ptr_value_type = typename std::pointer_traits<T>::element_type;

template <typename T>
using shared_ptr_of = std::shared_ptr<ptr_value_type<T>>;

template <typename T>
using unique_ptr_of = std::unique_ptr<ptr_value_type<T>, std::function<void(T)>>;

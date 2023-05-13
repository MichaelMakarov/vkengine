#pragma once
#include <functional>

template <typename>
class signal_function;

template <typename R, typename... Args>
class signal_function<R(Args...)> : public std::function<R(Args...)>
{
public:
    using std::function<R(Args...)>::function;
    R operator()(Args... args) const
    {
        if (*this)
        {
            std::function<R(Args...)>::operator()(std::forward<Args>(args)...);
        }
    }
};
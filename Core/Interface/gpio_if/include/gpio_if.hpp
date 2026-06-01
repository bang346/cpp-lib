#ifndef GPIO_IF_HPP
#define GPIO_IF_HPP

#include <stdint.h>

/// @brief Platform independent GPIO interface
struct gpio_if
{
    virtual ~gpio_if() = default;

    /// @brief Set GPIO High
    virtual void set_high() = 0;

    /// @brief Set GPIO Low
    virtual void set_low() = 0;
};

#endif

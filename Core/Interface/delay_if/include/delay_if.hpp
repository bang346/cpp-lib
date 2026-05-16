#ifndef DELAY_IF_HPP
#define DELAY_IF_HPP

#include <stdint.h>

/// @brief Platofrm independent delay
struct delay_if
{
    virtual ~delay_if() = default;

    /// @brief Blocking delay in ms
    /// @param time_ms  Delay in ms
    virtual void ms(uint32_t time_ms) = 0;
};

#endif

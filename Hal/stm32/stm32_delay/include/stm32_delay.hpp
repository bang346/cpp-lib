#ifndef STM32_DELAY_HPP
#define STM32_DELAY_HPP

#include <stdint.h>

#include "delay_if.hpp"
#include "stm32g4xx_hal.h"

/// @brief STM32 Delay implementation
class stm32_delay : public delay_if
{
public:
    /// @brief Delay in ms
    /// @param time_ms
    void ms(uint32_t time_ms) override
    {
        HAL_Delay(time_ms);
    }
};

#endif

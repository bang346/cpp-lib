#ifndef STM32_GPIO_HPP
#define STM32_GPIO_HPP

#include <stdint.h>

#include "gpio_if.hpp"
#include "stm32g4xx_hal.h"

/// @brief STM32 GPIO implementation
class stm32_gpio : public gpio_if
{
public:
    /// @brief Add GPIO
    /// @param port     GPIO Port
    /// @param pin      GPIO Pin
    stm32_gpio(GPIO_TypeDef *port, uint16_t pin)
        : port_{port}, pin_{pin}
    {
    }

    /// @brief Set GPIO High
    void set_high() override
    {
        HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
    }

    /// @brief Set GPIO Low
    void set_low() override
    {
        HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_RESET);
    }

private:
    GPIO_TypeDef *port_;
    uint16_t pin_;
};

#endif

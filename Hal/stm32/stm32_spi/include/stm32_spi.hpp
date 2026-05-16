#ifndef STM32_SPI_HPP
#define STM32_SPI_HPP

#include <stdint.h>

#include "bus_if.hpp"
#include "stm32g4xx_hal.h"

class stm32_spi : public bus_if
{
private:
    SPI_HandleTypeDef *const hspi_;
    GPIO_TypeDef *port_;
    const uint16_t pin_;

public:
    stm32_spi(SPI_HandleTypeDef *const hspi);
    stm32_spi(SPI_HandleTypeDef *const hspi, GPIO_TypeDef *port, const uint16_t &pin);

    virtual int transmit(const uint8_t *const data, const uint8_t len) const;
    virtual int receive(uint8_t *data, const uint8_t len) const;
    virtual int transmitreceive(uint8_t *const data_tx, uint8_t *data_rx, const uint8_t len) const;
};

#endif

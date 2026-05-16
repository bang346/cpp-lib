#ifndef STM32_SPI_HPP
#define STM32_SPI_HPP

#include "bus_if.hpp"

class stm32_spi : public bus_if
{
private:
    /* data */
public:
    virtual int transmit(const uint8_t *const data, const uint8_t len) const;
    virtual int receive(uint8_t *const data, const uint8_t len) const;
};




#endif
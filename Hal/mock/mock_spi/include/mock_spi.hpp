#ifndef MOCK_SPI_HPP
#define MOCK_SPI_HPP

#include <stdint.h>

#include "bus_if.hpp"
#include <gmock/gmock.h>

/// @brief STM32 Spi Interface
class mock_spi : public bus_if
{

public:
    MOCK_METHOD(int, transmit, (const uint8_t *const data, const uint8_t len), (const override));
    MOCK_METHOD(int, receive, (uint8_t *const data, const uint8_t len), (const override));
    MOCK_METHOD(int, transmitreceive, (uint8_t *const data_tx, uint8_t *data_rx, const uint8_t len), (const override));
};

#endif

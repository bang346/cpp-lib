#ifndef MOCK_BUS_HPP
#define MOCK_BUS_HPP

#include <stdint.h>
#include <gmock/gmock.h>

#include "bus_if.hpp"
#include "async_rx_bus_if.hpp"
#include "async_tx_bus_if.hpp"

/// @brief MOCK Spi Interface
class mock_bus
    : public bus_if,
      public async_rx_if,
      public async_tx_if
{

public:
    // Bus_if
    MOCK_METHOD(int, transmit, (const uint8_t *const data, const uint8_t len), (const override));
    MOCK_METHOD(int, receive, (uint8_t *const data, const uint8_t len), (const override));
    MOCK_METHOD(int, transmitreceive, (uint8_t *const data_tx, uint8_t *data_rx, const uint8_t len), (const override));

    // Bus_if_rx
    MOCK_METHOD(AsyncStartResult, start_receive, (std::uint8_t *destination, std::size_t capacity), (override));
    MOCK_METHOD(bool, is_receive_active, (), (const override));
    MOCK_METHOD(bool, take_receive_result, (AsyncResult & resul), (override));
    MOCK_METHOD(bool, abort_receive, (), (override));

    // Bus_if_tx
    MOCK_METHOD(AsyncStartResult, start_transmit, (const std::uint8_t *source, std::size_t length), (override));
    MOCK_METHOD(bool, is_transmit_active, (), (const override));
    MOCK_METHOD(bool, take_transmit_result, (AsyncResult & result), (override));
    MOCK_METHOD(bool, abort_transmit, (), (override));
};

#endif

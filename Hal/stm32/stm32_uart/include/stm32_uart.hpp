#ifndef STM32_UART_HPP
#define STM32_UART_HPP

#include <cstdint>
#include "stm32g4xx_hal.h"

#include "bus_if.hpp"

class stm32_uart_blocking : public bus_if
{
private:
    const std::uint32_t timeout_;
    UART_HandleTypeDef *huart_;

public:
    stm32_uart_blocking(const std::uint32_t &timeout, UART_HandleTypeDef *huart)
        : timeout_{timeout},
          huart_{huart}
    {
    }
    virtual ~stm32_uart_blocking() = default;
    /// @brief Method prototype to send data
    /// @param data[in] Data wich will be send
    /// @param len  Length of the data
    /// @return     0 = success
    virtual int transmit(const uint8_t *const data, const uint8_t len) const override
    {
        return HAL_UART_Transmit(huart_, data, len, timeout_);
    }

    /// @brief Method prototype to receive data
    /// @param data[out]    Pointer to the data buffer
    /// @param len  Length of the data
    /// @return     0 = success
    virtual int receive(uint8_t *const data, const uint8_t len) const override
    {
        return HAL_UART_Receive(huart_, data, len, timeout_);
    }

    /// @brief Method prototype to receive and send data
    /// @param data_tx[out] Pointer to the data out buffer
    /// @param data_rx[in]  Pointer to the data in buffer
    /// @param len          Length of the data
    /// @return
    virtual int transmitreceive(uint8_t *const data_tx, uint8_t *data_rx, const uint8_t len) const override
    {
        auto ret = transmit(data_tx, len);
        ret += receive(data_rx, len);
        return ret;
    }
};

#endif

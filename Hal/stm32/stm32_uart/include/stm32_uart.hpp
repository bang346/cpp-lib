#ifndef STM32_UART_HPP
#define STM32_UART_HPP

#include <cstdint>
#include <array>

#include "etl/vector.h"
#include "stm32g4xx_hal.h"

#include "bus_if.hpp"
#ifdef __cplusplus
extern "C"
{
#endif

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
            if ((data == nullptr) || (len == 0U))
            {
                return -1;
            }

            std::uint16_t received = 0U;

            const HAL_StatusTypeDef status =
                HAL_UARTEx_ReceiveToIdle(
                    huart_,
                    data,
                    static_cast<std::uint16_t>(len),
                    &received,
                    timeout_);

            if (status == HAL_OK)
            {
                return static_cast<int>(received);
            }

            // Auch bei einem Timeout können bereits Bytes empfangen worden sein.
            if ((status == HAL_TIMEOUT) && (received > 0U))
            {
                return static_cast<int>(received);
            }

            return -1;
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

    template <std::size_t size>
    class stm32_uart_dma : public bus_if
    {
    private:
        UART_HandleTypeDef *huart_;
        etl::vector<uint8_t, size> buffer_;
        int received_;

    public:
        stm32_uart_dma(UART_HandleTypeDef *huart)
            : huart_{huart},
              buffer_{},
              received_{}
        {
        }

        void start_uart_rx()
        {
            HAL_UARTEx_ReceiveToIdle_DMA(
                &huart4,
                rx_buffer,
                sizeof(rx_buffer));

            // Optional: kein Callback bei halbem Buffer
            __HAL_DMA_DISABLE_IT(huart4.hdmarx, DMA_IT_HT);
        }
        virtual ~stm32_uart_dma() = default;

        virtual int transmit(const uint8_t *const data, const uint8_t len) const override
        {
            int consumed = 0;
            if (received_ > len)
            {
                consumed = len;
                received_ -= len;
            }
            else
            {
                consumed = received_;
                received_ = 0;
            }

            for (size_t i = 0; i < consumed; i++)
            {
                data[i] = buffer_[i];
            }
            buffer_.erase(
                buffer_.begin(),
                buffer_.begin() + consumed);
            return consumed;
        }

        /// @brief Method prototype to receive data
        /// @param data[out]    Pointer to the data buffer
        /// @param len  Length of the data
        /// @return     0 = success
        virtual int receive(uint8_t *const data, const uint8_t len) const override
        {
        }

        /// @brief Method prototype to receive and send data
        /// @param data_tx[out] Pointer to the data out buffer
        /// @param data_rx[in]  Pointer to the data in buffer
        /// @param len          Length of the data
        /// @return
        virtual int transmitreceive(uint8_t *const data_tx, uint8_t *data_rx, const uint8_t len) const override
        {
        }
    };

#ifdef __cplusplus
}
#endif
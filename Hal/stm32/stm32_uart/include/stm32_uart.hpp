#ifndef STM32_UART_HPP
#define STM32_UART_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "stm32g4xx_hal.h"

#include "async_rx_bus_if.hpp"
#include "async_tx_bus_if.hpp"
#include "async_types.hpp"
#include "bus_if.hpp"

class stm32_uart final
    : public bus_if,
      public async_rx_if,
      public async_tx_if
{
private:
    const std::uint32_t timeout_;
    UART_HandleTypeDef *huart_;

    // ---------------------------------------------------------
    // TX async state
    // ---------------------------------------------------------

    std::atomic<bool> transmission_running_{false};
    std::atomic<bool> transmission_new_result_{false};

    AsyncResult transmit_result_{};

    std::size_t transmit_expected_length_{0U};

    // ---------------------------------------------------------
    // RX async state
    // ---------------------------------------------------------

    std::atomic<bool> receive_running_{false};
    std::atomic<bool> receive_new_result_{false};

    AsyncResult receive_result_{};

public:
    stm32_uart(const std::uint32_t &timeout, UART_HandleTypeDef *huart)
        : timeout_{timeout},
          huart_{huart}
    {
    }

    ~stm32_uart() override = default;

    // =========================================================
    // Blocking interface
    // =========================================================

    /// @brief Blocking UART transmission
    /// @return 0 = success, < 0 = error
    int transmit(const std::uint8_t *const data, const std::uint8_t len) const override
    {
        if ((data == nullptr) || (len == 0U))
        {
            return -1;
        }

        const HAL_StatusTypeDef status =
            HAL_UART_Transmit(huart_, data, len, timeout_);

        return status == HAL_OK ? 0 : -1;
    }

    /// @brief Blocking UART receive
    /// @return 0 = success, < 0 = error
    int receive(
        std::uint8_t *const data,
        const std::uint8_t len) const override
    {
        if ((data == nullptr) || (len == 0U))
        {
            return -1;
        }

        const HAL_StatusTypeDef status =
            HAL_UART_Receive(
                huart_,
                data,
                len,
                timeout_);

        return status == HAL_OK ? 0 : -1;
    }

    /// @brief Blocking transmit followed by receive
    int transmitreceive(
        std::uint8_t *const data_tx,
        std::uint8_t *data_rx,
        const std::uint8_t len) const override
    {
        const int tx_result = transmit(data_tx, len);

        if (tx_result != 0)
        {
            return tx_result;
        }

        return receive(data_rx, len);
    }

    // =========================================================
    // Async TX
    // =========================================================

    /// @brief Starts asynchronous UART DMA transmission
    ///
    /// @warning source must remain valid and unchanged until
    ///          transmission completes or is aborted.
    AsyncStartResult start_transmit(const std::uint8_t *source, std::size_t length) override
    {
        if ((source == nullptr) || (length == 0U))
        {
            return AsyncStartResult::InvalidArgument;
        }

        if (length > std::numeric_limits<std::uint16_t>::max())
        {
            return AsyncStartResult::InvalidArgument;
        }

        // Do not start a new transfer while one is running or
        // while the previous result has not been consumed.
        if (transmission_running_.load() ||
            transmission_new_result_.load())
        {
            return AsyncStartResult::Busy;
        }

        transmit_result_ = {};
        transmit_expected_length_ = length;

        const HAL_StatusTypeDef status =
            HAL_UART_Transmit_DMA(
                huart_,
                source,
                static_cast<std::uint16_t>(length));

        if (status == HAL_BUSY)
        {
            transmit_expected_length_ = 0U;
            return AsyncStartResult::Busy;
        }

        if (status != HAL_OK)
        {
            transmit_expected_length_ = 0U;

            transmit_result_.event = AsyncEvent::Error;
            transmit_result_.hardware_error =
                static_cast<std::uint32_t>(status);

            return AsyncStartResult::HardwareError;
        }

        transmission_running_.store(true);

        return AsyncStartResult::Started;
    }

    [[nodiscard]]
    bool is_transmit_active() const override
    {
        return transmission_running_.load();
    }

    /// @brief Gets a completed TX result exactly once.
    ///
    /// @return true  Result was available and written to result.
    /// @return false No new result available.
    bool take_transmit_result(
        AsyncResult &result) override
    {
        if (!transmission_new_result_.exchange(false))
        {
            return false;
        }

        result = transmit_result_;

        return true;
    }

    /// @brief Abort active asynchronous TX transfer.
    bool abort_transmit() override
    {
        if (!transmission_running_.load())
        {
            return false;
        }

        const HAL_StatusTypeDef status =
            HAL_UART_AbortTransmit(huart_);

        if (status != HAL_OK)
        {
            return false;
        }

        transmission_running_.store(false);

        transmit_result_.event = AsyncEvent::Aborted;
        transmit_result_.transferred_bytes = 0U;
        transmit_result_.hardware_error = 0U;

        transmit_expected_length_ = 0U;

        transmission_new_result_.store(true);

        return true;
    }

    /// @brief Called from HAL_UART_TxCpltCallback().
    void on_transmit_complete()
    {
        transmit_result_.event =
            AsyncEvent::Completed;

        transmit_result_.transferred_bytes =
            transmit_expected_length_;

        transmit_result_.hardware_error = 0U;

        transmit_expected_length_ = 0U;

        transmission_running_.store(false);

        // Set pending flag last.
        transmission_new_result_.store(true);
    }

    // =========================================================
    // Async RX
    // =========================================================

    /// @brief Starts asynchronous UART DMA reception.
    ///
    /// Reception ends when:
    /// - UART IDLE is detected
    /// - destination buffer is completely filled
    /// - an error occurs
    /// - reception is aborted
    ///
    /// @warning destination must remain valid until reception
    ///          completes or is aborted.
    AsyncStartResult start_receive(
        std::uint8_t *destination,
        std::size_t capacity) override
    {
        if ((destination == nullptr) || (capacity == 0U))
        {
            return AsyncStartResult::InvalidArgument;
        }

        if (capacity > std::numeric_limits<std::uint16_t>::max())
        {
            return AsyncStartResult::InvalidArgument;
        }

        // Previous result should first be consumed.
        if (receive_running_.load() ||
            receive_new_result_.load())
        {
            return AsyncStartResult::Busy;
        }

        receive_result_ = {};

        const HAL_StatusTypeDef status =
            HAL_UARTEx_ReceiveToIdle_DMA(
                huart_,
                destination,
                static_cast<std::uint16_t>(capacity));

        if (status == HAL_BUSY)
        {
            return AsyncStartResult::Busy;
        }

        if (status != HAL_OK)
        {
            receive_result_.event =
                AsyncEvent::Error;

            receive_result_.hardware_error =
                static_cast<std::uint32_t>(status);

            return AsyncStartResult::HardwareError;
        }

        // We do not need half-transfer events for this interface.
        // A ReceiveResult should only appear on IDLE or TC.
        if (huart_->hdmarx != nullptr)
        {
            __HAL_DMA_DISABLE_IT(
                huart_->hdmarx,
                DMA_IT_HT);
        }

        receive_running_.store(true);

        return AsyncStartResult::Started;
    }

    /// @brief Shows whether asynchronous RX is currently active.
    [[nodiscard]]
    bool is_receive_active() const override
    {
        return receive_running_.load();
    }

    /// @brief Gets a completed RX result exactly once.
    ///
    /// @return true  Result was available and written to result.
    /// @return false No new result available.
    bool take_receive_result(
        AsyncResult &result) override
    {
        if (!receive_new_result_.exchange(false))
        {
            return false;
        }

        result = receive_result_;

        return true;
    }

    /// @brief Abort active asynchronous RX transfer.
    bool abort_receive() override
    {
        if (!receive_running_.load())
        {
            return false;
        }

        const HAL_StatusTypeDef status =
            HAL_UART_AbortReceive(huart_);

        if (status != HAL_OK)
        {
            return false;
        }

        receive_running_.store(false);

        receive_result_.event =
            AsyncEvent::Aborted;

        receive_result_.transferred_bytes = 0U;
        receive_result_.hardware_error = 0U;

        receive_new_result_.store(true);

        return true;
    }

    /// @brief Called from HAL_UARTEx_RxEventCallback().
    ///
    /// @param size Number of received bytes reported by HAL.
    void on_receive_event(
        const std::uint16_t size)
    {
        const HAL_UART_RxEventTypeTypeDef event_type =
            HAL_UARTEx_GetRxEventType(huart_);

        switch (event_type)
        {
        case HAL_UART_RXEVENT_IDLE:

            receive_result_.event =
                AsyncEvent::Idle;

            break;

        case HAL_UART_RXEVENT_TC:

            receive_result_.event =
                AsyncEvent::Completed;

            break;

        case HAL_UART_RXEVENT_HT:

            // We don't treat half-transfer as a completed
            // receive operation.
            return;

        default:
            return;
        }

        receive_result_.transferred_bytes =
            static_cast<std::size_t>(size);

        receive_result_.hardware_error = 0U;

        receive_running_.store(false);

        // Set pending flag last.
        receive_new_result_.store(true);
    }

    // =========================================================
    // Error handling
    // =========================================================

    /// @brief Called from HAL_UART_ErrorCallback().
    void on_error()
    {
        const std::uint32_t error =
            HAL_UART_GetError(huart_);

        if (receive_running_.load())
        {
            receive_result_.event =
                AsyncEvent::Error;

            receive_result_.transferred_bytes = 0U;
            receive_result_.hardware_error = error;

            receive_running_.store(false);
            receive_new_result_.store(true);
        }

        if (transmission_running_.load())
        {
            transmit_result_.event =
                AsyncEvent::Error;

            transmit_result_.transferred_bytes = 0U;
            transmit_result_.hardware_error = error;

            transmit_expected_length_ = 0U;

            transmission_running_.store(false);
            transmission_new_result_.store(true);
        }
    }

    /// @brief Access underlying HAL handle.
    ///
    /// Mainly useful for mapping global HAL callbacks
    /// to the corresponding C++ UART instance.
    [[nodiscard]]
    UART_HandleTypeDef *handle() const noexcept
    {
        return huart_;
    }
};

/*
===============================================================================
Example HAL callbacks
===============================================================================

Assume that an instance exists somewhere with static/global lifetime:

    stm32_uart uart4{100U, &huart4};


-------------------------------------------------------------------------------
Transmit complete
-------------------------------------------------------------------------------

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart == uart4.handle())
    {
        uart4.on_transmit_complete();
    }
}


-------------------------------------------------------------------------------
Receive event

Called by HAL_UARTEx_ReceiveToIdle_DMA() on:
    - IDLE
    - Transfer Complete
    - potentially Half Transfer

on_receive_event() distinguishes between these event types.
-------------------------------------------------------------------------------

void HAL_UARTEx_RxEventCallback(
    UART_HandleTypeDef* huart,
    uint16_t Size)
{
    if (huart == uart4.handle())
    {
        uart4.on_receive_event(Size);
    }
}


-------------------------------------------------------------------------------
UART error
-------------------------------------------------------------------------------

void HAL_UART_ErrorCallback(
    UART_HandleTypeDef* huart)
{
    if (huart == uart4.handle())
    {
        uart4.on_error();
    }
}

===============================================================================
*/

#endif // STM32_UART_HPP
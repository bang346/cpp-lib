#ifndef STM32_SPI_HPP
#define STM32_SPI_HPP

#include <stdint.h>

#include "bus_if.hpp"
#include "stm32g4xx_hal.h"

/// @brief STM32 Spi Interface
class stm32_spi : public bus_if
{
private:
    static constexpr uint16_t HW_CS_IN_USE_ = 0xffff;   // Pin will get this value, when HW-CS is used

private: 
    SPI_HandleTypeDef *const hspi_;                     // STM32 HSPI Pointer
    GPIO_TypeDef *port_;                                // GPIO-Port
    const uint16_t pin_;                                // GPIO-Pin

public:
    /// @brief Constructor
    /// @note           Use this, when HW-CS is configured
    /// @param hspi     STM32 HSPI Pointer
    stm32_spi(SPI_HandleTypeDef *const hspi);

    /// @brief Constructor
    /// @param hspi     STM32 HSPI Pointer
    /// @param port     CS-GPIO-Port
    /// @param pin      CS-GPIO-Pin
    stm32_spi(SPI_HandleTypeDef *const hspi, GPIO_TypeDef *port, const uint16_t &pin);

    /// @brief Method prototype to send data
    /// @param data[in] Data wich will be send
    /// @param len  Length of the data
    /// @return     0 = success
    virtual int transmit(const uint8_t *const data, const uint8_t len) const override;

    /// @brief Method prototype to receive data
    /// @param data[out]    Pointer to the data buffer
    /// @param len  Length of the data
    /// @return     0 = success
    virtual int receive(uint8_t *const data, const uint8_t len) const override;

    /// @brief Method prototype to receive and send data
    /// @param data_tx[out] Pointer to the data out buffer
    /// @param data_rx[in]  Pointer to the data in buffer
    /// @param len          Length of the data
    /// @return
    virtual int transmitreceive(uint8_t *const data_tx, uint8_t *data_rx, const uint8_t len) const override;
};

#endif

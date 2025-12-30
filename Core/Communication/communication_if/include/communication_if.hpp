#ifndef COMMUNICATION_IF_HPP
#define COMMUNICATION_IF_HPP

#include <stdint.h>

/// @brief Pure Virtual Interface Class for communication
/// @details    Class must be overwritten for example SPI, UART, etc
class communication_if
{
private:
    /* data */
public:
    /// @brief Method prototype to send data
    /// @param data[in] Data wich will be send
    /// @param len  Length of the data
    /// @return     0 = success
    virtual int transmit(const uint8_t *const data, const uint8_t len) const = 0;

    /// @brief Method prototype to receive data
    /// @param data[out] Pointer to the data buffer
    /// @param len  Length of the data
    /// @return     0 = success
    virtual int receive(uint8_t *const data, const uint8_t len) const = 0;
};

#endif
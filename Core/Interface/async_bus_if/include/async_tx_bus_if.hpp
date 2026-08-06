#ifndef ASYNC_TX_BUS_IF_HPP
#define ASYNC_TX_BUS_IF_HPP

#include <stdint.h>
#include "async_types.hpp"

class async_tx_if
{
public:
    virtual ~async_tx_if() = default;

    /// @brief Starts the transmission and locks the memory
    /// @warning                    Destionation pointer must be valid
    ///                             until a transmit is completed or
    ///                             aborted!
    /// @param destination          Memory destination
    /// @param capacity             Capacity of the memory block
    /// @return                     @see AsyncStartResult
    virtual AsyncStartResult start_transmit(
        const std::uint8_t *source,
        std::size_t length) = 0;

    /// @brief Shows wheather a transmission process is running
    /// @return                     true = running,
    ///                             false = not running
    [[nodiscard]] virtual bool is_transmit_active() const = 0;

    /// @brief
    /// @note           The method returns a value
    ///                 only once until the next result is obtained.
    /// @param result
    /// @return         true = result is new and valid
    ///                 false = result is not new or invalid
    virtual bool take_transmit_result(
        AsyncResult &result) = 0;

    /// @brief Abort the running transmission
    /// @return                     true = successfull,
    ///                             false = error
    virtual bool abort_transmit() = 0;
};

#endif

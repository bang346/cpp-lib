#ifndef ASYNC_RX_BUS_IF_HPP
#define ASYNC_RX_BUS_IF_HPP

#include <stdint.h>
#include "async_types.hpp"

class async_rx_if
{
public:
    virtual ~async_rx_if() = default;

    /// @brief Starts the receive and locks the memory
    /// @warning                    Destionation pointer must be valid
    ///                             until a receive is completed or
    ///                             aborted!
    /// @param destination          Memory destination
    /// @param capacity             Capacity of the memory block
    /// @return                     @see AsyncStartResult
    virtual AsyncStartResult start_receive(
        std::uint8_t *destination,
        std::size_t capacity) = 0;

    /// @brief Shows wheather a receive process is running
    /// @return                     true = running,
    ///                             false = not running
    [[nodiscard]] virtual bool is_receive_active() const = 0;

    /// @brief
    /// @param result
    /// @return
    virtual bool take_receive_result(
        AsyncResult &result) = 0;

    /// @brief Abort the running receive
    /// @return                     true = successfull,
    ///                             false = error
    virtual bool abort_receive() = 0;
};

#endif

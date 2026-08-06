#ifndef ASYNC_RXTX_BUS_IF_HPP
#define ASYNC_RXTX_BUS_IF_HPP

#include <stdint.h>
#include "async_types.hpp"

class async_transfer_if
{
public:
    virtual ~async_transfer_if() = default;

    virtual AsyncStartResult start_transfer(
        const std::uint8_t *tx_data,
        std::uint8_t *rx_data,
        std::size_t length) = 0;

    [[nodiscard]]
    virtual bool is_transfer_active() const = 0;

    virtual bool take_transfer_result(
        AsyncResult &result) = 0;

    virtual bool abort_transfer() = 0;
};

#endif

#pragma once

#include <cstdint>
#include <limits>

#include "BusMasterReceiveCore.hpp"
#include "bus_if.hpp"

template <std::size_t Size>
class BusMasterReceiveSync
    : private BusMasterReceiveCore<Size>
{
private:
    using Core = BusMasterReceiveCore<Size>;

    bus_if &bus_;

public:
    explicit BusMasterReceiveSync(bus_if &bus)
        : bus_{bus}
    {
    }

    Frame_NS::CommError receive_raw(MessageId &id,
                                    std::uint8_t *output,
                                    Frame_NS::size_type output_capacity,
                                    Frame_NS::size_type &output_size,
                                    Frame_NS::frame_unpack_if &frame,
                                    Frame_NS::size_type expected_length)
    {
        // if (!output || output_capacity == 0U)
        // {
        //     return Frame_NS::
        // }
    }
};
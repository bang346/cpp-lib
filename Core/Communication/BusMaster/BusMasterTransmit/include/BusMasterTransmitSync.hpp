#pragma once

#include <cstdint>
#include <limits>

#include "BusMasterTransmitCore.hpp"
#include "bus_if.hpp"

/// @brief Asynchonous transmit spezialisation
/// @tparam Size            Internal buffer sizes
template <std::size_t Size>
class BusMasterTransmitSync final
    : private BusMasterTransmitCore<Size>
{
private:
    using Core = BusMasterTransmitCore<Size>;
    bus_if &bus_;

public:
    /// @brief Constructor
    /// @param receiver         transmit-obj
    explicit BusMasterTransmitSync(bus_if &bus)
        : bus_{bus}
    {
    }
    /// @brief Method wich will transmit a message
    /// @tparam Message             Message type
    /// @param message              Message wich will be send as payload
    /// @param frame                Frame object for frame format
    /// @return                     @see CommError
    template <typename Message>
    Frame_NS::CommError transmit(Message &message,
                                 Frame_NS::frame_pack_if &frame)
    {
        typename Core::EncodedFrame encoded{};

        const auto encode_result = this->encode(message, frame, encoded);
        if (encode_result != Frame_NS::CommError::None)
        {
            return encode_result;
        }

        // The existing bus_if uses uint8_t for the transfer length.
        if (encoded.size > static_cast<std::size_t>(std::numeric_limits<std::uint8_t>::max()))
        {
            return Frame_NS::CommError::InvalidLength;
        }

        const int transport_result = bus_.transmit(
            encoded.data,
            static_cast<std::uint8_t>(encoded.size));

        return transport_result == 0
                   ? Frame_NS::CommError::None
                   : Frame_NS::CommError::HardwareError;
    }
};
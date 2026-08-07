#pragma once

#include <cstddef>

#include "BusMasterTransmitCore.hpp"
#include "async_tx_bus_if.hpp"

/// @brief Asynchonous transmit spezialisation
/// @tparam Size            Internal buffer sizes
template <std::size_t Size>
class BusMasterTransmitAsync final
    : private BusMasterTransmitCore<Size>
{
private:
    using Core = BusMasterTransmitCore<Size>;

    async_tx_if &transmitter_;
    bool active_{false};
    std::size_t expected_length_{0U};

public:

    /// @brief Constructor
    /// @param transmitter          transmitter-obj
    explicit BusMasterTransmitAsync(async_tx_if &transmitter)
        : transmitter_{transmitter}
    {
    }

    /// @brief Method to start (DMA-) transmit
    /// @tparam Message             Message type
    /// @param message              Message wich will be send as payload
    /// @param frame                Frame object for frame format
    /// @return                     @see Frame_NS::CommError
    template <typename Message>
    Frame_NS::CommError start_transmit(
        Message &message,
        Frame_NS::frame_pack_if &frame)
    {
        if (active_ || transmitter_.is_transmit_active())
        {
            return Frame_NS::CommError::Busy;
        }

        typename Core::EncodedFrame encoded{};
        const auto encode_result = this->encode(message, frame, encoded);
        if (encode_result != Frame_NS::CommError::None)
        {
            return encode_result;
        }

        switch (transmitter_.start_transmit(encoded.data, encoded.size))
        {
        case AsyncStartResult::Started:
            expected_length_ = encoded.size;
            active_ = true;
            return Frame_NS::CommError::None;

        case AsyncStartResult::Busy:
            return Frame_NS::CommError::Busy;

        case AsyncStartResult::InvalidArgument:
            return Frame_NS::CommError::InvalidArgument;

        case AsyncStartResult::HardwareError:
        default:
            return Frame_NS::CommError::HardwareError;
        }
    }

    /// @brief Method to check the transmit status
    /// @return             @see Frame_NS::CommError
    Frame_NS::CommError process_transmit()
    {
        AsyncResult result{};
        if (!transmitter_.take_transmit_result(result))
        {
            return active_
                       ? Frame_NS::CommError::Busy
                       : Frame_NS::CommError::None;
        }

        // A result releases ownership of the internal frame buffer.
        active_ = false;
        const std::size_t expected_length = expected_length_;
        expected_length_ = 0U;

        switch (result.event)
        {
        case AsyncEvent::Completed:
            return result.transferred_bytes == expected_length
                       ? Frame_NS::CommError::None
                       : Frame_NS::CommError::HardwareError;

        case AsyncEvent::Aborted:
            // CommError currently has no dedicated Aborted value.
            return Frame_NS::CommError::Aborted;

        case AsyncEvent::Error:
        case AsyncEvent::Idle: // Idle is not a valid TX completion event.
        case AsyncEvent::None:
        default:
            return Frame_NS::CommError::HardwareError;
        }
    }

    /// @brief Method to abort the current transmission
    /// @return             true = success,
    ///                     false = error
    bool abort_transmit()
    {
        if (!active_)
        {
            return false;
        }

        if (!transmitter_.abort_transmit())
        {
            return false;
        }

        active_ = false;
        expected_length_ = 0U;
        return true;
    }

        
    /// @brief Getter active
    /// @return             active_
    [[nodiscard]]
    bool active() const noexcept
    {
        return active_;
    }
};
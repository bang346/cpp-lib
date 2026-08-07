#pragma once

#include <cstdint>
#include <limits>

#include "BusMasterReceiveCore.hpp"
#include "async_rx_bus_if.hpp"

template <std::size_t Size>
class BusMasterReceiveAsync final
    : private BusMasterReceiveCore<Size>
{
private:
    using Core = BusMasterReceiveCore<Size>;

    async_rx_if &receiver_;
    bool active_{false};

public:
    explicit BusMasterReceiveAsync(async_rx_if &receiver)
        : receiver_{receiver}
    {
    }

    Frame_NS::CommError start_receive(std::size_t maximum_length)
    {
        typename Core::ReceiveArea encoded{};

        // Check wheather a receive is running
        if (receiver_.is_receive_active() || active_)
        {
            return Frame_NS::CommError::Busy;
        }

        this->prepare_receive_area(maximum_length);

        // Check Pointer maximum_length <= Buffer.available && prepare frame_buffer_
        if (!static_cast<bool>(encoded))
        {
            return Frame_NS::CommError::ClassInternalBufferTooSmall; // frame_buffer_ inside Core
        }

        auto result = receiver_.start_receive(encoded.data, encoded.capacity);
        switch (result)
        {
        case AsyncStartResult::Busy:
            this->rollback_receive();
            return Frame_NS::CommError::Busy;
            break;
        case AsyncStartResult::Started:
            break;
        case AsyncStartResult::InvalidArgument:
            this->rollback_receive();
            return Frame_NS::CommError::InvalidArgument;
            break;
        case AsyncStartResult::HardwareError:
            this->rollback_receive();
            return Frame_NS::CommError::HardwareError;
            break;
        default:
            this->rollback_receive();
            return Frame_NS::CommError::ERROR_IS_UNDEFINED__;
            break;
        }
        active_ = true;
        return Frame_NS::CommError::None;
    }

    Frame_NS::CommError process_receive(
        MessageId &id,
        std::uint8_t *output,
        std::size_t output_capacity,
        std::size_t &output_size,
        Frame_NS::frame_unpack_if &frame)
    {
        if (receiver_.is_receive_active())
        {
            return Frame_NS::CommError::Busy;
        }
        if (!output || output_capacity == 0U)
        {
            return (!output) ? Frame_NS::CommError::InvalidArgument : Frame_NS::CommError::InvalidLength;
        }

        if (!this->receive_area_prepared())
        {
            return Frame_NS::CommError::InvalidState;
        }

        AsyncResult result;
        receiver_.take_receive_result(result);
        if (static_cast<bool>(result))
        {
            if (!this->commit_receive(result.transferred_bytes))
            {
                return Frame_NS::CommError::ClassInternalBufferTooSmall;
            }
            return this->check(id, output, output_capacity, output_size, frame);
        }

        // Error handling
        switch (result.event)
        {
        case AsyncEvent::Aborted:
            return Frame_NS::CommError::Aborted;
        case AsyncEvent::Error:
            return Frame_NS::CommError::HardwareError;
        default:
            break;
        }
        return Frame_NS::CommError::ERROR_IS_UNDEFINED__;
    }

    void abort_receive()
    {
        receiver_.abort_receive();
    }

    [[nodiscard]] bool active() const noexcept
    {
        return active_;
    }
};
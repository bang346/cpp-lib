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
        if (maximum_length == 0U)
        {
            return Frame_NS::CommError::InvalidLength;
        }

        // Check wheather a receive is running
        if (active_ || receiver_.is_receive_active())
        {
            return Frame_NS::CommError::Busy;
        }

        const typename Core::ReceiveArea area =
            this->prepare_receive_area(maximum_length);

        if (!static_cast<bool>(area))
        {
            return Frame_NS::CommError::ClassInternalBufferTooSmall;
        }

        switch (receiver_.start_receive(area.data, area.capacity))
        {
        case AsyncStartResult::Started:
            active_ = true;
            return Frame_NS::CommError::None;

        case AsyncStartResult::Busy:
            this->rollback_receive();
            return Frame_NS::CommError::Busy;

        case AsyncStartResult::InvalidArgument:
            this->rollback_receive();
            return Frame_NS::CommError::InvalidLength;

        case AsyncStartResult::HardwareError:
        default:
            this->rollback_receive();
            return Frame_NS::CommError::HardwareError;
        }
    }

    Frame_NS::CommError process_receive(
        MessageId &id,
        std::uint8_t *output,
        std::size_t output_capacity,
        std::size_t &output_size,
        Frame_NS::frame_unpack_if &frame)
    {
        output_size = 0U;

        if (output == nullptr || output_capacity == 0U)
        {
            return Frame_NS::CommError::InvalidLength;
        }

        AsyncResult result{};
        if (!receiver_.take_receive_result(result))
        {
            return active_
                       ? Frame_NS::CommError::Busy
                       : Frame_NS::CommError::None;
        }

        active_ = false;

        switch (result.event)
        {
        case AsyncEvent::Completed:
        case AsyncEvent::Idle:
            if (!this->receive_area_prepared())
            {
                return Frame_NS::CommError::InvalidState;
            }

            if (!this->commit_receive(result.transferred_bytes))
            {
                return Frame_NS::CommError::ClassInternalBufferTooSmall;
            }

            if (this->buffered_size() == 0U)
            {
                return Frame_NS::CommError::Timeout;
            }

            return this->check(
                id,
                output,
                output_capacity,
                output_size,
                frame);

        case AsyncEvent::Aborted:
            // Current public CommError has no dedicated Aborted state.
            this->rollback_receive();
            return Frame_NS::CommError::Aborted;

        case AsyncEvent::Error:
        case AsyncEvent::None:
        default:
            this->rollback_receive();
            return Frame_NS::CommError::HardwareError;
        }
    }

    bool abort_receive()
    {
        if (!active_)
        {
            return false;
        }

        if (!receiver_.abort_receive())
        {
            return false;
        }

        this->rollback_receive();
        active_ = false;
        return true;
    }

    [[nodiscard]] bool active() const noexcept
    {
        return active_;
    }
};

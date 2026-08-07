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

    /// @brief Read the message directly
    /// @tparam Message             Message-template
    /// @param message              Message obj
    /// @param bus_interface        Bus interface (uart)
    /// @param [inout] frame        Frame Version format
    /// @return                     @see Frame_NS::CommError
    template <typename Message>
    Frame_NS::CommError receive_message(Message &message,
                                        Frame_NS::frame_unpack_if &frame)
    {
        MessageId id;
        std::size_t len = 0;
        std::size_t expected_len = MessageTraits<Message>::maximumSize + frame.get_MaxSize();
        const auto result = receive_raw(id, message_buffer_.data(), message_buffer_.size(), len, frame, expected_len);

        if (result == Frame_NS::CommError::message_finished_buffer_not_empty || result == Frame_NS::CommError::None)
        {
            if (MessageTraits<Message>::maximumSize == len && MessageTraits<Message>::id == id)
            {
                BinaryReader reader(message_buffer_.data(), message_buffer_.size());
                message.serialize(reader);
            }
            else
            {
                return Frame_NS::CommError::serialize_failed;
            }
        }
        return result;
    }

    Frame_NS::CommError receive_raw(MessageId &id,
                                    std::uint8_t *output,
                                    Frame_NS::size_type output_capacity,
                                    Frame_NS::size_type &output_size,
                                    Frame_NS::frame_unpack_if &frame,
                                    Frame_NS::size_type expected_length)
    {
        // Check arguments
        if (!output || output_capacity == 0U || expected_length == 0U)
        {
            return (!output) ? Frame_NS::CommError::InvalidArgument : Frame_NS::CommError::InvalidLength;
        }

        typename Core::ReceiveArea encoded{};

        // Check wheather frame_buffer_ inside Core was prepared
        if (!this->receive_area_prepared())
        {
            encoded = this->prepare_receive_area(expected_length);
        }

        if (!static_cast<bool>(encoded))
        {
            return Frame_NS::CommError::ClassInternalBufferTooSmall; // frame_buffer_ inside Core
        }

        const int receivedLength = bus_.receive(encoded.data, encoded.capacity);
        if (receivedLength < 0)
        {
            this->rollback_receive();
            return Frame_NS::CommError::HardwareError;
        }
        else if (receivedLength == 0)
        {
            this->rollback_receive();
            return Frame_NS::CommError::Timeout;
        }
        if (!this->commit_receive(receivedLength)) // Checks internally wheather the received bytes fit into the frame_buffer_ inside Core
        {
            this->rollback_receive();
            return Frame_NS::CommError::ClassInternalBufferTooSmall; // More bytes received than the frame_buffer_ inside Core can hold
        }
        return this->check(id, output, output_capacity, output_size, frame);
    }
};
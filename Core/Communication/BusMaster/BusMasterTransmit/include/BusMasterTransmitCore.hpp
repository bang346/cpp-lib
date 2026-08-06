#pragma once
#include <cstddef>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "frame.hpp"
#include "binary_container.hpp"

template <std::size_t Size>
class BusMasterTransmitCore
{
    static_assert(Size > 0U, "BusMasterTransmitCore requires a non-zero buffer size");
    static_assert(Size <= static_cast<std::size_t>(std::numeric_limits<std::int16_t>::max()),
                  "frame_pack_if currently uses int16_t for outputCapacity");

protected:
    std::array<std::uint8_t, Size> message_buffer_{};
    std::array<std::uint8_t, Size> frame_buffer_{};

    struct EncodedFrame
    {
        const std::uint8_t *data{nullptr};
        std::size_t size{0U};
    };

    template <typename Message>
    Frame_NS::CommError encode(
        Message &message,
        Frame_NS::frame_pack_if &frame,
        EncodedFrame &encoded)
    {
        encoded = {};

        BinaryWriter writer(
            message_buffer_.data(),
            message_buffer_.size());

        message.serialize(writer);

        std::size_t frame_size = 0U;
        const Frame_NS::CommError encodeResult = frame.encode(MessageTraits<Message>::id,
                                                              message_buffer_.data(),
                                                              writer.size(),
                                                              frame_buffer_.data(),
                                                              frame_buffer_.size(),
                                                              frame_size);
        if (encodeResult != Frame_NS::CommError::None)
        {
            return encodeResult;
        }

        // Defensive check in case a frame implementation violates its contract.
        if (frame_size > frame_buffer_.size())
        {
            return Frame_NS::CommError::InvalidLength;
        }
        encoded.data = frame_buffer_.data();
        encoded.size = frame_size;
        return encodeResult;
    }
};
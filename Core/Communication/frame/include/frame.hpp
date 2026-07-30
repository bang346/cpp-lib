#ifndef FRAME_IF_HPP
#define FRAME_IF_HPP

#include <cstdint>
#include <cstddef>

#include "archive_defs.hpp"
#include "coder.hpp"

namespace Frame_NS
{
    enum class CommError : std::int16_t
    {
        None,
        Timeout,
        Busy,
        BufferTooSmall,
        ClassInternalBufferTooSmall,
        InvalidFrame,
        InvalidLength,
        CrcMismatch,
        UnsupportedVersion,
        HardwareError,
        Overflow,
        serialize_failed,
        message_unfinished
    };

    struct frame_pack_if
    {
        /// @brief Pure Virtual encode method
        /// @note                       1. The message must first be serialized using Binary...
        ///                             2. The class generates the final, unencoded / unencrypted message
        /// @details                    This method combines the payload with
        ///                             the frame and the checksum and creates
        ///                             an array that can then be sent.
        /// @param [in] messageid       Message id part from every frame
        /// @param payload              Byte array from BinaryWriter
        /// @param [in] payloadSize     Payload size
        /// @param output               Destination array
        /// @param [in] outputCapacity  Destination array capacity
        /// @param [out] outputSize     Finsihed message output size
        /// @return                     @see CommError
        virtual CommError encode(const MessageId &messageid,
                                 const std::uint8_t *payload,
                                 const std::size_t &payloadSize,
                                 std::uint8_t *output,
                                 const std::int16_t &outputCapacity,
                                 std::size_t &outputSize) const = 0;
    };

    struct frame_unpack_if
    {
        /// @brief Pure virtual decode method
        /// @param [out] messageid      MessageID used to create the correct message
        /// @param payload              Complete Received bytes
        /// @param [in] payloadSize     Size of the received message
        /// @param output               Buffer for the message (Real message without Frame, CRC)
        /// @param [in] outputCapacity  Size of the buffer
        /// @param [out] outputSize     Size of the message
        /// @return                     @see CommError
        virtual CommError decode(MessageId &messageid,
                                 const std::uint8_t *payload,
                                 const std::size_t &payloadSize,
                                 std::uint8_t *output,
                                 const std::int16_t &outputCapacity,
                                 std::size_t &outputSize) = 0;

        /// @brief Returns wheather the last decode call was sucessfull
        /// @return                 true = success,
        ///                         false = error
        virtual bool verify() const = 0;
    };

} // namespace Frame_NS

#endif

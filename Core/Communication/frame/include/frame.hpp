#ifndef FRAME_IF_HPP
#define FRAME_IF_HPP

#include <cstdint>
#include <cstddef>

#include "archive_defs.hpp"
#include "coder.hpp"

namespace Frame_NS
{
    using size_type = std::size_t;

    enum class CommError : std::int16_t
    {
        None,
        Timeout,
        Busy,
        BufferTooSmall,
        ClassInternalBufferTooSmall,
        InvalidFrame,
        InvalidLength,
        InvalidArgument,
        InvalidState,
        CrcMismatch,
        UnsupportedVersion,
        HardwareError,
        Overflow,
        serialize_failed,
        message_unfinished,
        message_finished_buffer_not_empty,
        Aborted,
        ERROR_IS_UNDEFINED__
    };

    enum class FrameVersion : uint8_t
    {
        undefinied,
        V1
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
                                 const size_type &payloadSize,
                                 std::uint8_t *output,
                                 const size_type &outputCapacity,
                                 size_type &outputSize) const = 0;
    };

    struct frame_unpack_if
    {
        /// @brief Pure virtual decode method
        /// @warning                    The inherting class must handle all listed return values!
        /// @param [out] messageid      MessageID used to create the correct message
        /// @param payload              Complete Received bytes
        /// @param [in] payloadSize     Size of the received message
        /// @param output               Buffer for the message (Real message without Frame, CRC)
        /// @param [in] outputCapacity  Size of the buffer
        /// @param [out] outputSize     Size of the message
        /// @return                     None = Message complete buffer empty,
        ///                             BufferTooSmall = outputCapacity > BinaryReader.size(),
        ///                             InvalidFrame = Startbyte and/or Version is wrong,
        ///                             BufferTooSmall = Destination buffer is to small
        ///                             message_unfinished = inputbuffer was read but the
        ///                             message is incomplete (used to generate the message with
        ///                             multiple calls),
        ///                             CrcMismatch = Checksum received and calculated are different
        ///                             message_finished_buffer_not_empty
        virtual CommError decode(MessageId &messageid,
                                 const std::uint8_t *payload,
                                 const size_type &payloadSize,
                                 std::uint8_t *output,
                                 const size_type &outputCapacity,
                                 size_type &outputSize) = 0;

        /// @brief Returns wheather the last decode call was sucessfull
        /// @return                 true = success,
        ///                         false = error
        virtual bool verify() const = 0;

        /// @brief Method for obtaining the payload
        ///                             index when the payload has not been
        ///                             read in its entirety and data is therefore still in the buffer
        /// @return                     Index payload
        virtual size_type get_index() const = 0;

        /// @brief Method to get the maximum frame size
        /// @return
        virtual size_type get_MaxSize() const = 0;
    };

} // namespace Frame_NS

#endif

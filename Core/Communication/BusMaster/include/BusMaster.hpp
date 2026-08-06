#ifndef BUS_MASTER_HPP
#define BUS_MASTER_HPP

#include <cstdint>
#include <array>
#include <cstring>
#include <chrono>
#include <iostream>

#include "bus_if.hpp"
#include "crc.hpp"
#include "archive_defs.hpp"
#include "coder_if.hpp"
#include "binary_container.hpp"
#include "frame.hpp"
#include "frameV1.hpp"

#include "etl/vector.h"
/*

Version V0.1
*/

namespace BusMaster_NS
{
    /*
        Zu Beachten empfangen
Byte verloren
zusätzliches Byte
CRC -
Fehler
unvollständiger Frame
Frame beginnt mitten im Datenstrom
Empfangsbuffer läuft über
Gerät wird während einer Nachricht eingeschaltet

    Zu beachten

    - Soll methoden zum direkten senden und paketweisen senden anbieten
    - Codierer Interface soll verfügbar sein
    - Abhängig vom System Container zum buffern oder Ringbuffer


Tests
Ein kompletter Frame wird korrekt dekodiert.
Der Frame wird Byte für Byte übergeben.
Zwei Frames kommen in einem Buffer.
Ein Frame kommt über mehrere Buffer verteilt.
Ein Byte fehlt.
Ein zusätzliches Byte befindet sich im Stream.
Die CRC ist falsch.
Das Längenfeld ist ungültig.
Der Payload ist maximal groß.
Der Empfangsbuffer läuft über.
Nach einem defekten Frame wird der nächste Frame erkannt.
Little- und Big-Endian-Werte werden korrekt behandelt.
Ein unbekannter Nachrichtentyp wird sauber abgewiesen.
Eine unbekannte Protokollversion wird erkannt.
    */

    struct FrameHeader
    {
        static constexpr std::uint8_t startbyte = 0xF7;
        MessageId messageid{};
        static constexpr std::uint8_t Version = 0;
        std::uint16_t PayloadLength{};
    };

    template <std::size_t size>
    class BusMasterTransmit
    {
    private:
        std::array<uint8_t, size> message_buffer_;
        std::array<uint8_t, size> frame_buffer;

    public:
        BusMasterTransmit(/* args */)
            : message_buffer_{},
              frame_buffer{}
        {
        }
        virtual ~BusMasterTransmit() = default;

        template <typename Message>
        Frame_NS::CommError transmit(Message &message, const bus_if *bus_interface, Frame_NS::frame_pack_if &frame)
        {

            std::size_t len = 0;
            BinaryWriter writer(
                message_buffer_.data(),
                message_buffer_.size());

            message.serialize(writer);

            const Frame_NS::CommError encodeResult = frame.encode(MessageTraits<Message>::id,
                                                                  message_buffer_.data(),
                                                                  writer.size(),
                                                                  frame_buffer.data(),
                                                                  frame_buffer.size(),
                                                                  len);
            if (encodeResult != Frame_NS::CommError::None)
            {
                return encodeResult;
            }

            bus_interface->transmit(frame_buffer.data(), len);
            return Frame_NS::CommError::None;
        }
    };

    /// @brief BusMaster Receive Class
    /// @tparam size    Internal storage size
    template <std::size_t size>
    class BusMasterReceive final
    {
    private:
        // std::array<uint8_t, size> frame_buffer;
        std::array<uint8_t, size> message_buffer_;
        etl::vector<std::uint8_t, 100> frame_buffer_;
        Frame_NS::FrameVersion FrameVersionIdentified_;
        bool VersionChecked_;
        std::size_t index_;

    public:
        BusMasterReceive(/* args */)
            : message_buffer_{},
              frame_buffer_{},
              FrameVersionIdentified_{Frame_NS::FrameVersion::undefinied},
              VersionChecked_{false},
              index_{0}
        {
        }
        virtual ~BusMasterReceive() = default;

        /// @brief Read the message directly
        /// @tparam Message             Message-template
        /// @param message              Message obj
        /// @param bus_interface        Bus interface (uart)
        /// @param [inout] frame        Frame Version format
        /// @return                     @see Frame_NS::CommError
        template <typename Message>
        Frame_NS::CommError receive_message(Message &message,
                                            const bus_if *bus_interface,
                                            Frame_NS::frame_unpack_if &frame)
        {
            MessageId id;
            std::size_t len = 0;
            std::size_t expected_len = MessageTraits<Message>::maximumSize + frame.get_MaxSize();
            const auto result = receive_raw(id, message_buffer_.data(), message_buffer_.size(), len, bus_interface, frame, expected_len);

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

        /// @brief Method to get the Paylaod
        /// @warning                    Currently supports only FrameV1!
        /// @note                       Use this method, when the message and
        ///                             unknown and length is known
        /// @param [out] id             Messageid
        /// @param [out] output         Output destination array
        /// @param [in] outputCapacity  Size of the array
        /// @param [out] outputSize     Received size
        /// @param bus_interface        Bus interface (uart)
        /// @param [inout] frame        Frame Version format
        /// @param [in] expected_len    How many bytes to read (important for linux systems)
        /// @return                     @see Frame_NS::CommError
        Frame_NS::CommError receive_raw(MessageId &id,
                                        std::uint8_t *output,
                                        const std::int16_t &outputCapacity,
                                        std::size_t &outputSize,
                                        const bus_if *bus_interface,
                                        Frame_NS::frame_unpack_if &frame,
                                        const std::size_t &expected_len)
        {
            using namespace Frame_NS;

            if ((bus_interface == nullptr) || (output == nullptr) ||
                (outputCapacity <= 0) || (expected_len == 0U))
            {
                return CommError::InvalidLength;
            }

            if (frame_buffer_.available() == 0)
            {
                frame_buffer_.clear();
                reset();
                return CommError::ClassInternalBufferTooSmall;
            }
            const std::size_t receive_capacity =
                std::min(expected_len, frame_buffer_.available());

            const std::size_t old_size = frame_buffer_.size();
            // Receive data

            frame_buffer_.uninitialized_resize(old_size + receive_capacity);
            const int receivedLength = bus_interface->receive(
                frame_buffer_.data() + old_size,
                receive_capacity);

            if (receivedLength < 0)
            {
                frame_buffer_.clear();
                reset();
                return CommError::HardwareError;
            }

            const std::size_t received_size =
                static_cast<std::size_t>(receivedLength);

            if (received_size > receive_capacity)
            {
                frame_buffer_.clear();
                reset();
                return CommError::InvalidLength;
            }

            // Discard the part that was reserved but not written.
            frame_buffer_.uninitialized_resize(old_size + received_size);

            if ((received_size == 0U) && (old_size == 0U))
            {
                return CommError::InvalidLength;
            }

            return check(id, output, outputCapacity, outputSize, frame);
        }

        /// @brief Method to get the Paylaod
        /// @warning                    Currently supports only FrameV1!
        /// @note                       Use this method, when the message and
        ///                             and length is unknown
        /// @param [out] id             Messageid
        /// @param [out] output         Output destination array
        /// @param [in] outputCapacity  Size of the array
        /// @param [out] outputSize     Received size
        /// @param bus_interface        Bus interface (uart)
        /// @param [inout] frame        Frame Version format
        /// @return                     @see Frame_NS::CommError
        Frame_NS::CommError receive_raw(MessageId &id,
                                        std::uint8_t *output,
                                        const std::int16_t &outputCapacity,
                                        std::size_t &outputSize,
                                        const bus_if *bus_interface,
                                        Frame_NS::frame_unpack_if &frame)
        {
            return receive_raw(id, output, outputCapacity, outputSize, bus_interface, frame, frame_buffer_.available());
        }

        /// @brief Method to check the remaining bytes from a message
        /// @note                       Internally used to check the new message
        ///                             (and the remaining bytes inside the buffer)
        /// @param [out] id             Messageid
        /// @param [out] output         Output destination array
        /// @param [in] outputCapacity  Size of the array
        /// @param [out] outputSize     Received size
        /// @param bus_interface        Bus interface (uart)
        /// @param [inout] frame        Frame Version format
        /// @return                     @see Frame_NS::CommError
        Frame_NS::CommError check(MessageId &id,
                                  std::uint8_t *output,
                                  const std::int16_t &outputCapacity,
                                  std::size_t &outputSize,
                                  Frame_NS::frame_unpack_if &frame)
        {
            using namespace Frame_NS;

            if (frame_buffer_.empty())
            {
                return CommError::InvalidLength;
            }

            const CommError decodeResult = frame.decode(
                id,
                frame_buffer_.data(),
                frame_buffer_.size(),
                output,
                outputCapacity,
                outputSize);

            switch (decodeResult)
            {
            case CommError::message_finished_buffer_not_empty:
            {
                const std::size_t consumed = frame.get_index();

                if ((consumed == 0U) || (consumed > frame_buffer_.size()))
                {
                    frame_buffer_.clear();
                    reset();
                    return CommError::InvalidLength;
                }

                frame_buffer_.erase(
                    frame_buffer_.begin(),
                    frame_buffer_.begin() + consumed);

                reset();
                return decodeResult;
            }

            case CommError::message_unfinished:
                // The unpacker stores the intermediate frame state internally.
                // Therefore all bytes passed to decode() have been consumed and
                // must not be submitted again on the next call.
                frame_buffer_.clear();
                return decodeResult;

            case CommError::None:
                frame_buffer_.clear();
                reset();
                return CommError::None;

            default:
                frame_buffer_.clear();
                reset();
                return decodeResult;
            }
        }

        void reset()
        {
            VersionChecked_ = false;
            FrameVersionIdentified_ = Frame_NS::FrameVersion::undefinied;
        }
    };

} // namespace BusMaster_NS

#endif

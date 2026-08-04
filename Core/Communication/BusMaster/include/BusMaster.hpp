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
        std::array<uint8_t, size> message_buffer;
        std::array<uint8_t, size> frame_buffer;

    public:
        BusMasterTransmit(/* args */)
            : message_buffer{},
              frame_buffer{}
        {
        }
        virtual ~BusMasterTransmit() = default;

        template <typename Message>
        Frame_NS::CommError transmit(Message &message, const bus_if *bus_interface, Frame_NS::frame_pack_if &frame)
        {

            std::size_t len = 0;
            BinaryWriter writer(
                message_buffer.data(),
                message_buffer.size());

            message.serialize(writer);

            const Frame_NS::CommError encodeResult = frame.encode(MessageTraits<Message>::id,
                                                                  message_buffer.data(),
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
    class BusMasterReceive
    {
    private:
        std::array<uint8_t, size> frame_buffer;
        std::array<uint8_t, size> message_buffer;
        Frame_NS::FrameVersion FrameVersionIdentified_;
        bool VersionChecked_;
        std::size_t index_;

    public:
        BusMasterReceive(/* args */)
            : frame_buffer{},
              message_buffer{},
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
            const auto result = receive_raw(id, message_buffer.data(), message_buffer.size(), len, bus_interface, frame, expected_len);

            if (result == Frame_NS::CommError::message_finished_buffer_not_empty || result == Frame_NS::CommError::None)
            {
                if (MessageTraits<Message>::maximumSize == len && MessageTraits<Message>::id == id)
                {
                    BinaryReader reader(message_buffer.data(), message_buffer.size());
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
            const std::size_t freeCapacity =  // When the last message was completed but the buffer wasnt empty
                frame_buffer.size() - index_; // the old data is written to the beginning of frame_buffer

            if (freeCapacity == 0)
            {
                index_ = 0;
                reset();
                return CommError::ClassInternalBufferTooSmall;
            }
            std::size_t actual_len = expected_len;
            if (expected_len > freeCapacity)
            {
                actual_len = freeCapacity;
            }
            // Receive data

            const int receivedLength = bus_interface->receive(
                frame_buffer.data() + index_,
                actual_len);

            if (receivedLength < 0)
            {
                index_ = 0;
                reset();
                return CommError::HardwareError;
            }

            if (receivedLength == 0 && index_ == 0)
            {
                return CommError::InvalidLength;
            }

            return check(id, output, outputCapacity, outputSize, frame, receivedLength);
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
            return receive_raw(id, output, outputCapacity, outputSize, bus_interface, frame, frame_buffer.size());
        }

        void reset()
        {
            VersionChecked_ = false;
            FrameVersionIdentified_ = Frame_NS::FrameVersion::undefinied;
        }

        /// @brief Method to check the remaining bytes from a message
        /// @note                       Use this message when a complete
        ///                             message is expected to be inside
        ///                             the framebuffer
        /// @param [out] id             Messageid
        /// @param [out] output         Output destination array
        /// @param [in] outputCapacity  Size of the array
        /// @param [out] outputSize     Received size
        /// @param bus_interface        Bus interface (uart)
        /// @param [inout] frame        Frame Version format
        /// @return                     @see Frame_NS::CommError
        virtual Frame_NS::CommError check(MessageId &id,
                                          std::uint8_t *output,
                                          const std::int16_t &outputCapacity,
                                          std::size_t &outputSize,
                                          Frame_NS::frame_unpack_if &frame)
        {
            return check(id, output, outputCapacity, outputSize, frame, 0);
        }

    private:
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
        virtual Frame_NS::CommError check(MessageId &id,
                                          std::uint8_t *output,
                                          const std::int16_t &outputCapacity,
                                          std::size_t &outputSize,
                                          Frame_NS::frame_unpack_if &frame,
                                          const int &receivedLength)
        {
            using namespace Frame_NS;
            const std::size_t totalSize =
                index_ + static_cast<std::size_t>(receivedLength);
            const CommError decodeResult = frame.decode(
                id,
                frame_buffer.data(),
                totalSize,
                output,
                outputCapacity,
                outputSize);

            switch (decodeResult)
            {
            case CommError::message_finished_buffer_not_empty:
            {

                const std::size_t consumed = frame.get_index();

                if (consumed > totalSize)
                {
                    index_ = 0;
                    reset();
                    return CommError::InvalidLength;
                }

                const std::size_t remaining =
                    totalSize - consumed;

                std::memmove(
                    frame_buffer.data(),
                    frame_buffer.data() + consumed,
                    remaining);

                index_ = remaining;
                reset();

                return decodeResult;
            }

            case CommError::message_unfinished:
                // Dein frameV1_unpack speichert den Zwischenzustand intern.
                // Daher sind alle übergebenen Bytes bereits verbraucht.
                index_ = 0;
                return decodeResult;

            case CommError::None:
                index_ = 0;
                reset();
                return CommError::None;

            default:
                index_ = 0;
                reset();
                return decodeResult;
            }
        }
    };

} // namespace BusMaster_NS

#endif

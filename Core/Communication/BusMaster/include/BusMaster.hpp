#ifndef BUS_MASTER_HPP
#define BUS_MASTER_HPP

#include <cstdint>
#include <array>

#include "bus_if.hpp"
#include "crc.hpp"
#include "archive_defs.hpp"
#include "coder_if.hpp"
#include "binary_container.hpp"

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

    enum class CommError : std::int16_t
    {
        None,
        Timeout,
        Busy,
        BufferTooSmall,
        InvalidFrame,
        InvalidLength,
        CrcMismatch,
        UnsupportedVersion,
        HardwareError,
        Overflow,
        serialize_failed
    };

    struct FrameHeader
    {
        static constexpr std::uint8_t startbyte = 0xF7;
        MessageId messageid{};
        static constexpr std::uint8_t Version = 0;
        std::uint16_t PayloadLength{};
    };

    template <typename std::size_t size>
    class BusMasterTransmit
    {
    private:
        std::array<uint8_t, size> buffer_;
        FrameHeader frame_;

    public:
        BusMasterTransmit(/* args */)
            : buffer_{},
              frame_{}
        {
        }
        virtual ~BusMasterTransmit() = default;

        template <typename Message>
        CommError transmit(Message &message, const bus_if *bus_interface, const coder_if *coder)
        {
            BinaryWriter writer(
                buffer_.data(),
                buffer_.size());

            frame_.messageid = MessageTraits<Message>::id;
            frame_.PayloadLength = MessageTraits<Message>::maximumSize;
            if (!writer.write(frame_.startbyte) || !writer.write(static_cast<std::uint16_t>(frame_.messageid)) || !writer.write(frame_.PayloadLength) || !writer.write(frame_.PayloadLength))
            {
                return CommError::BufferTooSmall;
            }

            if (!message.serialize(writer))
            {
                return CommError::BufferTooSmall;
            }

            auto new_len = coder->code(buffer_.data(), writer.size(), buffer_.data());
            bus_interface->transmit(buffer_.data(), new_len);
            return CommError::None;
        }
    };

} // namespace BusMaster_NS

#endif

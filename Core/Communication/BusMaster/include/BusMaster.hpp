#ifndef BUS_MASTER_HPP
#define BUS_MASTER_HPP

#include <cstdint>

#include "bus_if.hpp"
#include "crc.hpp"
#include "archive_defs.hpp"

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
        Overflow
    };

    struct FrameHeader
    {
        std::uint8_t startbyte{};
        MessageId messageid{};
        std::uint8_t Version{};
        std::uint16_t PayloadLength{};
    };

    struct Frame
    {
        FrameHeader header{};
        const std::uint8_t *Payload{};
    };

    class BusMasterTransmit
    {
    private:
        /* data */
    public:
        BusMasterTransmit(/* args */) = default;
        virtual ~BusMasterTransmit() = default;
    };

} // namespace BusMaster_NS

#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <type_traits>

// ============================================================
// Message-IDs
// ============================================================

enum class MessageId : std::uint16_t
{
    MotorCommand = 0x0010,
    SensorData = 0x0020
};

// ============================================================
// MessageTraits
//
// Das allgemeine Template ist absichtlich nicht definiert.
// Für jeden gültigen Nachrichtentyp gibt es eine Spezialisierung.
// ============================================================

template <typename Message>
struct MessageTraits;

// ============================================================
// BinaryWriter
// ============================================================

class BinaryWriter
{
public:
    BinaryWriter(std::uint8_t *buffer, std::size_t capacity)
        : buffer_(buffer),
          capacity_(capacity)
    {
    }

    template <typename... Types>
    bool operator()(Types &...values)
    {
        return (write(values) && ...);
    }

    template <typename T>
    bool write(T value)
    {
        static_assert(
            std::is_integral<T>::value,
            "BinaryWriter unterstützt nur Integer-Typen");

        if (position_ + sizeof(T) > capacity_)
        {
            return false;
        }

        using UnsignedT = typename std::make_unsigned<T>::type;

        const UnsignedT raw =
            static_cast<UnsignedT>(value);

        // Little Endian
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            buffer_[position_++] =
                static_cast<std::uint8_t>(
                    (raw >> (8U * i)) & 0xFFU);
        }

        return true;
    }

    std::size_t size() const
    {
        return position_;
    }

private:
    std::uint8_t *buffer_;
    std::size_t capacity_;
    std::size_t position_ = 0;
};

// ============================================================
// BinaryReader
// ============================================================

class BinaryReader
{
public:
    BinaryReader(
        const std::uint8_t *buffer,
        std::size_t size)
        : buffer_(buffer),
          size_(size)
    {
    }

    template <typename... Types>
    bool operator()(Types &...values)
    {
        return (read(values) && ...);
    }

    template <typename T>
    bool read(T &value)
    {
        static_assert(
            std::is_integral<T>::value,
            "BinaryReader unterstützt nur Integer-Typen");

        if (position_ + sizeof(T) > size_)
        {
            return false;
        }

        using UnsignedT = typename std::make_unsigned<T>::type;

        UnsignedT raw = 0;

        // Little Endian
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            raw |= static_cast<UnsignedT>(
                       buffer_[position_++])
                   << (8U * i);
        }

        value = static_cast<T>(raw);

        return true;
    }

    bool finished() const
    {
        return position_ == size_;
    }

private:
    const std::uint8_t *buffer_;
    std::size_t size_;
    std::size_t position_ = 0;
};

// ============================================================
// Nachricht 1: MotorCommand
// ============================================================

struct MotorCommand
{
    std::int16_t speedRpm = 0;
    std::int16_t torqueMilliNm = 0;
    std::uint8_t mode = 0;

    template <typename Archive>
    bool serialize(Archive &archive)
    {
        return archive(
            speedRpm,
            torqueMilliNm,
            mode);
    }
};

template <>
struct MessageTraits<MotorCommand>
{
    static constexpr MessageId id =
        MessageId::MotorCommand;

    static constexpr std::size_t maximumSize =
        sizeof(std::int16_t) +
        sizeof(std::int16_t) +
        sizeof(std::uint8_t);
};

// ============================================================
// Nachricht 2: SensorData
// ============================================================

struct SensorData
{
    std::int16_t temperatureCentiDegree = 0;
    std::uint16_t voltageMilliVolt = 0;
    std::uint32_t timestampMs = 0;

    template <typename Archive>
    bool serialize(Archive &archive)
    {
        return archive(
            temperatureCentiDegree,
            voltageMilliVolt,
            timestampMs);
    }
};

template <>
struct MessageTraits<SensorData>
{
    static constexpr MessageId id =
        MessageId::SensorData;

    static constexpr std::size_t maximumSize =
        sizeof(std::int16_t) +
        sizeof(std::uint16_t) +
        sizeof(std::uint32_t);
};

// ============================================================
// Generische Serialisierung
// ============================================================

template <typename Message, std::size_t BufferSize>
bool serializeMessage(
    Message &message,
    std::array<std::uint8_t, BufferSize> &buffer,
    std::size_t &serializedSize)
{
    static_assert(
        MessageTraits<Message>::maximumSize <= BufferSize,
        "Der übergebene Buffer ist zu klein");

    BinaryWriter writer(
        buffer.data(),
        buffer.size());

    if (!message.serialize(writer))
    {
        return false;
    }

    serializedSize = writer.size();

    return true;
}

// ============================================================
// Generische Deserialisierung
// ============================================================

template <typename Message>
bool deserializeMessage(
    Message &message,
    const std::uint8_t *data,
    std::size_t size)
{
    BinaryReader reader(data, size);

    if (!message.serialize(reader))
    {
        return false;
    }

    return reader.finished();
}

// ============================================================
// Hilfsfunktion zur Ausgabe
// ============================================================

void printBuffer(
    const std::uint8_t *data,
    std::size_t size)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<unsigned>(data[i])
            << ' ';
    }

    std::cout << std::dec << '\n';
}

// ============================================================
// Hauptprogramm
// ============================================================

int main()
{
    constexpr std::size_t BufferSize = 32;

    std::array<std::uint8_t, BufferSize> buffer{};

    // --------------------------------------------------------
    // MotorCommand serialisieren
    // --------------------------------------------------------

    MotorCommand motorTx{
        1500,
        320,
        2};

    std::size_t motorSize = 0;

    if (!serializeMessage(
            motorTx,
            buffer,
            motorSize))
    {
        std::cerr
            << "MotorCommand konnte nicht serialisiert werden\n";

        return 1;
    }

    std::cout << "MotorCommand\n";

    std::cout
        << "Message-ID: 0x"
        << std::hex
        << static_cast<std::uint16_t>(
               MessageTraits<MotorCommand>::id)
        << std::dec
        << '\n';

    std::cout << "Serialisierte Daten: ";
    printBuffer(buffer.data(), motorSize);

    MotorCommand motorRx{};

    if (!deserializeMessage(
            motorRx,
            buffer.data(),
            motorSize))
    {
        std::cerr
            << "MotorCommand konnte nicht deserialisiert werden\n";

        return 1;
    }

    std::cout
        << "Empfangen: speed="
        << motorRx.speedRpm
        << ", torque="
        << motorRx.torqueMilliNm
        << ", mode="
        << static_cast<unsigned>(motorRx.mode)
        << "\n\n";

    // --------------------------------------------------------
    // SensorData serialisieren
    // --------------------------------------------------------

    SensorData sensorTx{
        2350,
        48000,
        123456};

    std::size_t sensorSize = 0;

    if (!serializeMessage(
            sensorTx,
            buffer,
            sensorSize))
    {
        std::cerr
            << "SensorData konnte nicht serialisiert werden\n";

        return 1;
    }

    std::cout << "SensorData\n";

    std::cout
        << "Message-ID: 0x"
        << std::hex
        << static_cast<std::uint16_t>(
               MessageTraits<SensorData>::id)
        << std::dec
        << '\n';

    std::cout << "Serialisierte Daten: ";
    printBuffer(buffer.data(), sensorSize);

    SensorData sensorRx{};

    if (!deserializeMessage(
            sensorRx,
            buffer.data(),
            sensorSize))
    {
        std::cerr
            << "SensorData konnte nicht deserialisiert werden\n";

        return 1;
    }

    std::cout
        << "Empfangen: temperature="
        << sensorRx.temperatureCentiDegree
        << ", voltage="
        << sensorRx.voltageMilliVolt
        << ", timestamp="
        << sensorRx.timestampMs
        << '\n';

    return 0;
}
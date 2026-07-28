#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <cstdint>
#include <cstddef>
#include <array>

#include "binary_container.hpp"

/// @brief Function wich serializes an std::array
/// @tparam Message         Messsage
/// @see Core\Communication\archive\include\archive_defs.hpp
/// @tparam BufferSize      Size of the array must be smaller or
///                         equal to the size of the buffer
/// @param message          Message structure
/// @param buffer           External buffer to save data
/// @param serializedSize   Size of the serialized data
/// @return                 true = success,
///                         false = error
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

/// @brief Function to deserialize data
/// @tparam Message         Messsage
/// @see Core\Communication\archive\include\archive_defs.hpp
/// @param message          Message structure
/// @param data             Pointer to the data destination
/// @param size             How many are needed for a complete message
/// @return                 true = success,
///                         false = error
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

#endif

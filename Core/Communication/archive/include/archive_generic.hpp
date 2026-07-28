#ifndef ARCHIVE_GENERIC_HPP
#define ARCHIVE_GENERIC_HPP

#include <cstdint>
#include <array>

#include "archive_defs.hpp"

template <std::size_t Chunksize>
struct ConfigureGeneric
{
    std::uint16_t command;
    std::array<std::uint8_t, Chunksize> Payload;

    template <typename Archive>
    bool serialize(Archive &archive)
    {
        return archive(
            command,
            Payload);
    }
};

template <std::size_t ChunkSize>
struct MessageTraits<ConfigureGeneric<ChunkSize>>
{
    static constexpr MessageId id =
        MessageId::ConfigureGeneric;
    static constexpr std::size_t chunkSize =
        ChunkSize;

    static constexpr size_t maximumSize =
        sizeof(std::uint16_t) +
        ChunkSize;
};

#endif

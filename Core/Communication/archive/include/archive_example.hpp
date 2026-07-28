#ifndef ARCHIVE_TEMPLATE_HPP
#define ARCHIVE_TEMPLATE_HPP

#include <cstdint>
#include "archive_defs.hpp"

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

    static constexpr size_t maximumSize =
        sizeof(std::int16_t) +
        sizeof(std::int16_t) +
        sizeof(std::uint8_t);
};

#endif

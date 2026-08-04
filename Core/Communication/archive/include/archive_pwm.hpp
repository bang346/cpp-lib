#ifndef ARCHIVE_PWM_HPP
#define ARCHIVE_PWM_HPP

#include <cstdint>
#include <array>

#include "archive_defs.hpp"

enum class PWM_Command : uint8_t
{
    start = 1,
    stopp,
    overwrite,
    overwritestart
};

struct ConfigurePWMs
{
    std::uint32_t periode;
    std::uint8_t ch1;
    std::uint8_t ch2;
    std::uint8_t ch3;
    std::uint32_t Pulse1;
    std::uint32_t Pulse2;
    std::uint32_t Pulse3;

    template <typename Archive>
    bool serialize(Archive &archive)
    {
        return archive(
            periode,
            ch1,
            ch2,
            ch3,
            Pulse1,
            Pulse2,
            Pulse3);
    }
};

template <>
struct MessageTraits<ConfigurePWMs>
{
    static constexpr MessageId id =
        MessageId::ConfigurePWMs;

    static constexpr size_t maximumSize =
        sizeof(std::uint32_t) +
        3 * sizeof(PWM_Command) +
        3 * sizeof(std::uint32_t);
};

#endif

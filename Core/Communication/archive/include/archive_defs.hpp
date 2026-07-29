#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <stdint.h>

enum class MessageId : uint16_t
{
    MotorCommand = 0x0010,
    SensorData = 0x0020,
    ConfigureGeneric = 0x0030
};

template <typename Message>
struct MessageTraits;

#endif
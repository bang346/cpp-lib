#pragma once

#include <cstddef>
#include <cstdint>

enum class AsyncStartResult : std::uint8_t
{
    Started,
    Busy,
    InvalidArgument,
    HardwareError
};

enum class AsyncEvent : std::uint8_t
{
    None,
    Completed,
    Idle,
    Aborted,
    Error
};

struct AsyncResult
{
    std::size_t transferred_bytes{0U};
    AsyncEvent event{AsyncEvent::None};
    std::uint32_t hardware_error{0U};
};

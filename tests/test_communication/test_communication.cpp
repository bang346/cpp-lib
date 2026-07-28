#include <gtest/gtest.h>
#include <iostream>
#include <array>

#include "archive_example.hpp"
#include "archive_generic.hpp"
#include "serialize.hpp"

TEST(Communication, Serialize)
{
    // Create Buffer
    constexpr std::size_t BufferSize = 32;

    std::array<std::uint8_t, BufferSize> buffer{};

    ConfigureGeneric<2> data{
        1,
        {1, 2}};

    std::size_t motorsize = 0;

    if (!serializeMessage(data, buffer, motorsize))
    {
        std::cerr << "MotorCommand konnte nicht serialisiert werden\n";
    }

    int x = 0;
}
#include <gtest/gtest.h>
#include <iostream>
#include <array>

#include "BusMaster.hpp"
#include "crc.hpp"
#include "coder.hpp"
#include "mock_spi.hpp"
#include "archive_generic.hpp"
#include "archive_example.hpp"

using ::testing::_;
using ::testing::Invoke;

TEST(Communication, Serialize)
{
    // std::array<std::uint8_t, 5> payload{
    //     0xDC, 0x05,
    //     0x40, 0x01,
    //     0x02};

    // BusMaster_NS::Frame Frame =
    //     {
    //         {0xF7,
    //          MessageId::ConfigureGeneric,
    //          0,
    //          static_cast<std::uint16_t>(payload.size())},
    //         payload.data()};

    // const uint16_t crc16 = crc_compute<uint16_t>(
    //     payload.data(),
    //     payload.size(),
    //     0xFFFFu,
    //     0x1021u,
    //     CrcBitOrder::MsbFirst,
    //     0x0000u);

    // std::array<uint8_t, 64> buffer{};
    // uint32_t value = 0xFAFBFCFD;
    // using UnsignedT = typename std::make_unsigned<uint32_t>::type;

    // const UnsignedT raw =
    //     static_cast<UnsignedT>(value);

    // auto x = sizeof(uint32_t);
    // // Little Endian
    // for (std::size_t i = 0; i < sizeof(uint32_t); ++i)
    // {
    //     buffer[i] =
    //         static_cast<std::uint8_t>(
    //             (raw >> (8U * i)) & 0xFFU);
    // }

    // int beta = 0;
}

TEST(Communication, CoderCrC)
{
    std::array<std::uint8_t, 5> result_array;
    std::array<std::uint8_t, 5> payload{
        0xDC, 0x05,
        0x40, 0x01,
        0x02};

    coder_crc<std::uint16_t> coder(0xFFFFu, 0x1021u);

    uint8_t data[30] = {};
    uint8_t result[30] = {};
    auto len = coder.code(payload.data(), payload.size(), data);
    int x = 0;
    auto newlen = coder.decode(data, len, result_array.data());

    ASSERT_EQ(result_array, payload);
    ASSERT_TRUE(coder.decode_result());
    int z = 0;
}

TEST(Communication, Transmit)
{
    BusMaster_NS::BusMasterTransmit<32> DUT;
    coder_crc<std::uint16_t> coder(0xFFFFu, 0x1021u);
    mock_spi spi;

    std::vector<uint8_t> captured_data;
    uint8_t captured_len = 0;

    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};

    EXPECT_CALL(spi, transmit(_, _))
        .WillOnce(Invoke(
            [&](const uint8_t *data, const uint8_t len) -> int
            {
                captured_len = len;

                // Inhalt kopieren, damit er nach dem Aufruf noch verfügbar ist
                captured_data.assign(data, data + len);

                return 0;
            }));

    DUT.transmit(command, &spi, &coder);
}
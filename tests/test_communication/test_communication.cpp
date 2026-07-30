#include <gtest/gtest.h>
#include <iostream>
#include <array>
#include <type_traits>

#include "BusMaster.hpp"
#include "crc.hpp"
#include "coder.hpp"
#include "mock_spi.hpp"
#include "archive_generic.hpp"
#include "archive_example.hpp"
#include "frameV1.hpp"

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
    // std::array<std::uint8_t, 5> result_array;
    // std::array<std::uint8_t, 5> payload{
    //     0xDC, 0x05,
    //     0x40, 0x01,
    //     0x02};

    // coder_crc<std::uint16_t> coder(0xFFFFu, 0x1021u);

    // uint8_t data[30] = {};
    // uint8_t result[30] = {};
    // auto len = coder.code(payload.data(), payload.size(), data);
    // int x = 0;
    // auto newlen = coder.decode(data, len, result_array.data());

    // ASSERT_EQ(result_array, payload);
    // ASSERT_TRUE(coder.decode_result());
    // int z = 0;
}

// TEST(Communication, Transmit)
// {
//     BusMaster_NS::BusMasterTransmit<32> DUT;
//     coder_crc<std::uint16_t> coder(0xFFFFu, 0x1021u);
//     mock_spi spi;

//     std::vector<uint8_t> captured_data;
//     uint8_t captured_len = 0;

//     ConfigureGeneric<12> command{
//         0xfA,
//         {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};

//     EXPECT_CALL(spi, transmit(_, _))
//         .WillOnce(Invoke(
//             [&](const uint8_t *data, const uint8_t len) -> int
//             {
//                 captured_len = len;

//                 // Inhalt kopieren, damit er nach dem Aufruf noch verfügbar ist
//                 captured_data.assign(data, data + len);

//                 return 0;
//             }));

//     DUT.transmit(command, &spi, &coder);
// }

template <typename>
struct extract_argument;

template <template <typename> class ClassType, typename T>
struct extract_argument<ClassType<T>>
{
    using type = T;
};

TEST(Communication, FramePack)
{
    std::array<uint8_t, 100> buffer{};
    std::array<uint8_t, 100> buffer_message{};
    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);

    frameV1_pack frame(coder);

    BinaryWriter write(buffer_message.data(), buffer_message.size());

    std::size_t len = 0;
    command.serialize(write);
    auto result = frame.encode(MessageTraits<ConfigureGeneric<12>>::id, buffer_message.data(), write.size(), buffer.data(), buffer.size(), len);
    int x = 0;

    // Test
    ASSERT_EQ(frameV1_header::startbyte, buffer[0]);
    ASSERT_EQ(frameV1_header::version_, buffer[1]);
    uint16_t id = (buffer[2] | buffer[3] << 8);
    ASSERT_EQ(static_cast<uint16_t>(MessageTraits<ConfigureGeneric<12>>::id), id);
    ASSERT_EQ(frameV1_header::FrameHeadSize_ + frame.crc_size + sizeof(command), buffer[4]);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    for (size_t i = 0; i < write.size(); i++)
    {
        ASSERT_EQ(buffer_message[i], buffer[i + frameV1_header::FrameHeadSize_]);
    }
    auto crc = coder.code(buffer.data() + 1, len - 1 - frame.crc_size);
    uint16_t crc_msg = (buffer[20] << 8) | buffer[19];
    ASSERT_EQ(crc_msg, crc);
}

TEST(Communication, FrameUnpack)
{
    std::array<uint8_t, 100> buffer{};
    std::array<uint8_t, 100> buffer_message{};
    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);

    frameV1_pack frame(coder);

    BinaryWriter write(buffer_message.data(), buffer_message.size());

    std::size_t len = 0;
    command.serialize(write);
    auto result = frame.encode(MessageTraits<ConfigureGeneric<12>>::id, buffer_message.data(), write.size(), buffer.data(), buffer.size(), len);
    int x = 0;

    frameV1_unpack DUT(coder);

    MessageId id;

    std::array<std::uint8_t, 100> decoded_message{};
    result = DUT.decode(id, buffer.data(), len, decoded_message.data(), decoded_message.size(), len);

    BinaryReader reader(decoded_message.data(), len);
    ConfigureGeneric<12> message{};

    message.serialize(reader);

    int y = 0;
}
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
#include "mock_spi.hpp"

using ::testing::_;
using ::testing::Invoke;

TEST(Communication, BasciSendReceive)
{
    std::vector<std::uint8_t> data_tx_received;
    // CRC16
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);

    // Init Frame
    frameV1_pack frame_pack(coder);
    frameV1_unpack frame_unpack(coder);
    // Mock for test (SPI mock is the same for uart)
    mock_spi mock;

    EXPECT_CALL(mock, transmit(_, _))
        .WillOnce(Invoke([&data_tx_received](const uint8_t *const data, const uint8_t len) -> int
                         {
                            EXPECT_NE(data, nullptr);
                            EXPECT_GT(len, 0);
                            data_tx_received.assign(data, data + len);

                            EXPECT_EQ(data_tx_received.size(), len);
                            EXPECT_EQ(data_tx_received[0], data[0]);
                            return 0; }));

    EXPECT_CALL(mock, receive(_, _))
        .WillOnce(Invoke([&data_tx_received](uint8_t *const data, const uint8_t len) -> int
                         {
                            for (size_t i = 0; i < data_tx_received.size(); i++)
                            {
                                data[i] = data_tx_received[i];
                            }
                            
                        return data_tx_received.size(); }));

    // Transmit Part
    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};
    BusMaster_NS::BusMasterTransmit<32> DUT1;
    BusMaster_NS::BusMasterReceive<32> DUT2;

    ConfigureGeneric<12> command_rx{};
    auto result = DUT1.transmit(command, &mock, frame_pack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    // Receive Part
    MessageId id;

    std::array<uint8_t, 32> receive_buffer{};
    std::size_t len = 0;
    result = DUT2.receive_raw(id, receive_buffer.data(), receive_buffer.size(), len, &mock, frame_unpack);

    ASSERT_EQ(result, Frame_NS::CommError::None);
    if (MessageTraits<ConfigureGeneric<12>>::id == id)
    {
        BinaryReader reader(receive_buffer.data(), receive_buffer.size());
        command_rx.serialize(reader);
    }
}

TEST(Communication, ReceiveMessage)
{
    std::vector<std::uint8_t> data_tx_received;
    // CRC16
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);

    // Init Frame
    frameV1_pack frame_pack(coder);
    frameV1_unpack frame_unpack(coder);
    // Mock for test (SPI mock is the same for uart)
    mock_spi mock;

    EXPECT_CALL(mock, transmit(_, _))
        .WillOnce(Invoke([&data_tx_received](const uint8_t *const data, const uint8_t len) -> int
                         {
                            EXPECT_NE(data, nullptr);
                            EXPECT_GT(len, 0);
                            data_tx_received.assign(data, data + len);

                            EXPECT_EQ(data_tx_received.size(), len);
                            EXPECT_EQ(data_tx_received[0], data[0]);
                            return 0; }));

    EXPECT_CALL(mock, receive(_, _))
        .WillRepeatedly(Invoke([&data_tx_received](uint8_t *const data, const uint8_t len) -> int
                               {
                            for (size_t i = 0; i < data_tx_received.size(); i++)
                            {
                                data[i] = data_tx_received[i];
                            }
                            
                        return data_tx_received.size(); }));

    // Transmit Part
    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};
    BusMaster_NS::BusMasterTransmit<32> DUT1;
    BusMaster_NS::BusMasterReceive<32> DUT2;

    ConfigureGeneric<12> command_rx{};
    auto result = DUT1.transmit(command, &mock, frame_pack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    // Receive Part
    MessageId id;

    std::array<uint8_t, 32> receive_buffer{};
    std::size_t len = 0;
    result = DUT2.receive_message(command_rx, &mock, frame_unpack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    ConfigureGeneric<11> command_rx_error{};
    MotorCommand command_rx_error2;
    result = DUT2.receive_message(command_rx_error, &mock, frame_unpack);
    ASSERT_EQ(result, Frame_NS::CommError::serialize_failed);
    result = DUT2.receive_message(command_rx_error2, &mock, frame_unpack);
    ASSERT_EQ(result, Frame_NS::CommError::serialize_failed);
}

TEST(Communication, ReceiveMessagePartialy)
{
    std::vector<std::uint8_t> data_tx_received;
    // CRC16
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    std::vector<std::size_t> sizes{20, 3, 2, 4, 3, 10};
    std::size_t index = 0;
    // Init Frame
    frameV1_pack frame_pack(coder);
    frameV1_unpack frame_unpack(coder);
    // Mock for test (SPI mock is the same for uart)
    mock_spi mock;

    EXPECT_CALL(mock, transmit(_, _))
        .WillOnce(Invoke([&data_tx_received](const uint8_t *const data, const uint8_t len) -> int
                         {
                            EXPECT_NE(data, nullptr);
                            EXPECT_GT(len, 0);
                            data_tx_received.assign(data, data + len);

                            EXPECT_EQ(data_tx_received.size(), len);
                            EXPECT_EQ(data_tx_received[0], data[0]);
                            return 0; }));

    EXPECT_CALL(mock, receive(_, _))
        .WillRepeatedly(Invoke([&](uint8_t *const data, const uint8_t len) -> int
                               {
                                if(sizes.size() == 0)
                                {
                                    return 0;
                                }
                                auto thissize = sizes.back();
                                
                                sizes.pop_back();
                                for (size_t i = 0; i < thissize; i++)
                                {
                                    data[i] = data_tx_received[i+index];
                                }
                                index += thissize;
                            
                        return thissize; }));

    // Transmit Part
    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};
    BusMaster_NS::BusMasterTransmit<32> DUT1;
    BusMaster_NS::BusMasterReceive<32> DUT2;

    ConfigureGeneric<12> command_rx{};
    auto result = DUT1.transmit(command, &mock, frame_pack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    data_tx_received.insert(data_tx_received.end(), data_tx_received.begin(), data_tx_received.begin() + 21);

    // Receive Part
    result = Frame_NS::CommError::message_unfinished;

    while (result != Frame_NS::CommError::None)
    {
        result = DUT2.receive_message(command_rx, &mock, frame_unpack);
        if (result == Frame_NS::CommError::message_finished_buffer_not_empty)
        {
            int i = 0;
        }
    }

    ASSERT_EQ(result, Frame_NS::CommError::None);
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

#include <gtest/gtest.h>
#include <iostream>
#include <array>
#include <type_traits>
#include <deque>

#include "BusMasterReceiveSync.hpp"
#include "BusMasterTransmitSync.hpp"
#include "crc.hpp"
#include "coder.hpp"
#include "mock_spi.hpp"
#include "archive_generic.hpp"
#include "archive_example.hpp"
#include "frameV1.hpp"
#include "mock_spi.hpp"
#include "archive_pwm.hpp"

using ::testing::_;
using ::testing::Invoke;

TEST(BusMaster, BasciSendReceive)
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
    BusMasterTransmitSync<32> DUT1(mock);
    BusMasterReceiveSync<32> DUT2(mock);

    ConfigureGeneric<12> command_rx{};
    auto result = DUT1.transmit(command, frame_pack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    // Receive Part
    MessageId id;

    std::array<uint8_t, 32> receive_buffer{};
    std::size_t len = 0;
    result = DUT2.receive_raw(id, receive_buffer.data(), receive_buffer.size(), len, frame_unpack, receive_buffer.size());

    ASSERT_EQ(result, Frame_NS::CommError::None);
    if (MessageTraits<ConfigureGeneric<12>>::id == id)
    {
        BinaryReader reader(receive_buffer.data(), receive_buffer.size());
        command_rx.serialize(reader);
    }
}

TEST(BusMaster, ReceiveMessage)
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
    BusMasterTransmitSync<32> DUT1(mock);
    BusMasterReceiveSync<32> DUT2(mock);

    ConfigureGeneric<12> command_rx{};
    auto result = DUT1.transmit(command, frame_pack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    // Receive Part
    MessageId id;

    std::array<uint8_t, 32> receive_buffer{};
    std::size_t len = 0;
    result = DUT2.receive_message(command_rx, frame_unpack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    ConfigureGeneric<11> command_rx_error{};
    MotorCommand command_rx_error2;
    result = DUT2.receive_message(command_rx_error, frame_unpack);
    ASSERT_EQ(result, Frame_NS::CommError::ClassInternalBufferTooSmall);
    result = DUT2.receive_message(command_rx_error2, frame_unpack);
    ASSERT_EQ(result, Frame_NS::CommError::ClassInternalBufferTooSmall);
}

TEST(BusMaster, ReceiveMessagePartialy)
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
    BusMasterTransmitSync<32> DUT1(mock);
    BusMasterReceiveSync<32> DUT2(mock);

    ConfigureGeneric<12> command_rx{};
    auto result = DUT1.transmit(command, frame_pack);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    data_tx_received.insert(data_tx_received.end(), data_tx_received.begin(), data_tx_received.begin() + 21);

    // Receive Part
    result = Frame_NS::CommError::message_unfinished;

    while (result != Frame_NS::CommError::None)
    {
        result = DUT2.receive_message(command_rx, frame_unpack);
        if (result == Frame_NS::CommError::message_finished_buffer_not_empty)
        {
            int i = 0;
        }
    }

    ASSERT_EQ(result, Frame_NS::CommError::None);
}

TEST(BusMaster, BusMasterMessage)
{
    std::array<uint8_t, 100> buffer{};
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    frameV1_pack frame_pack(coder);
    frameV1_unpack frame_unpack(coder);

    mock_spi mock;

    std::deque<std::uint8_t> data_tx_rx;
    EXPECT_CALL(mock, transmit(_, _))
        .WillRepeatedly(Invoke([&](const uint8_t *const data, const uint8_t len) -> int
                               {
                            EXPECT_NE(data, nullptr);
                            EXPECT_GT(len, 0);
                            for (size_t i = 0; i < len; i++)
                            {
                                data_tx_rx.push_back(data[i]);
                            }
                            return 0; }));

    EXPECT_CALL(mock, receive(_, _))
        .WillRepeatedly(Invoke([&](uint8_t *const data, const uint8_t len) -> int
                               {
                                int ret = data_tx_rx.size();

                                for (size_t i = 0; i < ret; i++)
                                {
                                    data[i] = data_tx_rx.front();
                                    data_tx_rx.pop_front();
                                    
                                }
                                
                            
                        return ret; }));

    BusMasterTransmitSync<100> DUT1(mock);
    BusMasterReceiveSync<100> DUT2(mock);

    ConfigurePWMs PWMs;
    PWMs.ch1 = (uint8_t)PWM_Command::start;
    PWMs.ch2 = (uint8_t)PWM_Command::stopp;
    PWMs.ch3 = (uint8_t)PWM_Command::overwritestart;
    PWMs.periode = 0xff00;
    PWMs.Pulse1 = 0xf0f0;
    PWMs.Pulse2 = 0xf0f1;
    PWMs.Pulse3 = 0xf0f3;

    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};

    DUT1.transmit(PWMs, frame_pack);
    DUT1.transmit(command, frame_pack);

    // Receive Part
    ConfigurePWMs PWMs_received{};
    ConfigureGeneric<12> configuregeneric{};
    MessageId id;
    std::size_t size = 0;
    Frame_NS::CommError result;
    do
    {
        result = DUT2.receive_raw(id, buffer.data(), buffer.size(), size, frame_unpack, buffer.size());
        if (id == MessageTraits<ConfigurePWMs>::id)
        {
            BinaryReader reader(buffer.data(), buffer.size());
            PWMs_received.serialize(reader);
            for (size_t i = 0; i < sizeof(PWMs_received); i++)
            {
                EXPECT_EQ(PWMs_received.ch1, PWMs.ch1);
                EXPECT_EQ(PWMs_received.ch2, PWMs.ch2);
                EXPECT_EQ(PWMs_received.ch3, PWMs.ch3);
                EXPECT_EQ(PWMs_received.periode, PWMs.periode);
                EXPECT_EQ(PWMs_received.Pulse1, PWMs.Pulse1);
                EXPECT_EQ(PWMs_received.Pulse2, PWMs.Pulse2);
                EXPECT_EQ(PWMs_received.Pulse3, PWMs.Pulse3);
            }
        }
        else if (id == MessageTraits<ConfigureGeneric<12>>::id)
        {
            BinaryReader reader(buffer.data(), buffer.size());
            configuregeneric.serialize(reader);
            EXPECT_EQ(configuregeneric.command, command.command);
            for (size_t i = 0; i < sizeof(configuregeneric.Payload); i++)
            {
                EXPECT_EQ(configuregeneric.Payload[i], command.Payload[i]);
            }
        }
    } while (result == Frame_NS::CommError::message_finished_buffer_not_empty);
}

TEST(BusMaster, Check)
{

    std::array<uint8_t, 100> buffer{};
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    frameV1_pack frame_pack(coder);
    frameV1_unpack frame_unpack(coder);

    mock_spi mock;

    std::deque<std::uint8_t> data_tx_rx;
    EXPECT_CALL(mock, transmit(_, _))
        .WillRepeatedly(Invoke([&](const uint8_t *const data, const uint8_t len) -> int
                               {
                            EXPECT_NE(data, nullptr);
                            EXPECT_GT(len, 0);
                            for (size_t i = 0; i < len; i++)
                            {
                                data_tx_rx.push_back(data[i]);
                            }
                            return 0; }));

    EXPECT_CALL(mock, receive(_, _))
        .WillRepeatedly(Invoke([&](uint8_t *const data, const uint8_t len) -> int
                               {
                                int ret = data_tx_rx.size();

                                for (size_t i = 0; i < ret; i++)
                                {
                                    data[i] = data_tx_rx.front();
                                    data_tx_rx.pop_front();

                                }

                        return ret; }));

    BusMasterTransmitSync<100> DUT1(mock);
    BusMasterReceiveSync<100> DUT2(mock);

    ConfigurePWMs PWMs;
    PWMs.ch1 = (uint8_t)PWM_Command::start;
    PWMs.ch2 = (uint8_t)PWM_Command::stopp;
    PWMs.ch3 = (uint8_t)PWM_Command::overwritestart;
    PWMs.periode = 0xff00;
    PWMs.Pulse1 = 0xf0f0;
    PWMs.Pulse2 = 0xf0f1;
    PWMs.Pulse3 = 0xf0f3;

    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};

    DUT1.transmit(PWMs, frame_pack);
    DUT1.transmit(command, frame_pack);

    // Receive Part
    ConfigurePWMs PWMs_received{};
    ConfigureGeneric<12> configuregeneric{};
    MessageId id;
    std::size_t size = 0;
    Frame_NS::CommError result = DUT2.receive_raw(id, buffer.data(), buffer.size(), size, frame_unpack, buffer.size());

    // if (result == Frame_NS::CommError::message_finished_buffer_not_empty)
    // {
    //     result = DUT2.check(id, buffer.data(), buffer.size(), size, frame_unpack);
    //     int i = 0;
    //     EXPECT_EQ(result, Frame_NS::CommError::None);
    // }
}

TEST(BusMaster, CheckErrors)
{

    std::array<uint8_t, 100> buffer{};
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    frameV1_pack frame_pack(coder);
    frameV1_unpack frame_unpack(coder);

    std::deque<int> results = {-1, 0, 40};
    mock_spi mock;

    EXPECT_CALL(mock, receive(_, _))
        .WillRepeatedly(Invoke([&](uint8_t *const data, const uint8_t len) -> int
                               {
                                int ret = results.front();
                                results.pop_front();

                        return ret; }));

    BusMasterReceiveSync<10> DUT2(mock);

    MessageId id;
    std::size_t size = 0;
    EXPECT_EQ(DUT2.receive_raw(id, buffer.data(), buffer.size(), size, frame_unpack, buffer.size()), Frame_NS::CommError::HardwareError);
    EXPECT_EQ(DUT2.receive_raw(id, buffer.data(), buffer.size(), size, frame_unpack, buffer.size()), Frame_NS::CommError::Timeout);
    EXPECT_EQ(DUT2.receive_raw(id, buffer.data(), buffer.size(), size, frame_unpack, buffer.size()), Frame_NS::CommError::ClassInternalBufferTooSmall);
}

template <typename>
struct extract_argument;

template <template <typename> class ClassType, typename T>
struct extract_argument<ClassType<T>>
{
    using type = T;
};

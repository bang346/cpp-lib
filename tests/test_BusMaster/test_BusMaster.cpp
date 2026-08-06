#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "BusMasterReceiveCore.hpp"
#include "BusMasterTransmitAsync.hpp"
#include "BusMasterTransmitCore.hpp"
#include "BusMasterTransmitSync.hpp"

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
    struct TestMessage
    {
        std::uint16_t value{0U};

        void serialize(BinaryWriter &writer)
        {
            writer(value);
        }
    };
} // namespace

template <>
struct MessageTraits<TestMessage>
{
    static constexpr MessageId id = MessageId::MotorCommand;
    static constexpr std::size_t maximumSize = sizeof(std::uint16_t);
};

namespace
{
    class MockFramePack : public Frame_NS::frame_pack_if
    {
    public:
        MOCK_METHOD(
            Frame_NS::CommError,
            encode,
            (const MessageId &message_id,
             const std::uint8_t *payload,
             const std::size_t &payload_size,
             std::uint8_t *output,
             const std::int16_t &output_capacity,
             std::size_t &output_size),
            (const, override));
    };

    class MockFrameUnpack : public Frame_NS::frame_unpack_if
    {
    public:
        MOCK_METHOD(
            Frame_NS::CommError,
            decode,
            (MessageId & message_id,
             const std::uint8_t *payload,
             const std::size_t &payload_size,
             std::uint8_t *output,
             const std::int16_t &output_capacity,
             std::size_t &output_size),
            (override));

        MOCK_METHOD(bool, verify, (), (const, override));
        MOCK_METHOD(std::size_t, get_index, (), (const, override));
        MOCK_METHOD(std::size_t, get_MaxSize, (), (const, override));
    };

    class MockBus : public bus_if
    {
    public:
        MOCK_METHOD(
            int,
            transmit,
            (const std::uint8_t *const data, const std::uint8_t len),
            (const, override));

        MOCK_METHOD(
            int,
            receive,
            (std::uint8_t *const data, const std::uint8_t len),
            (const, override));

        MOCK_METHOD(
            int,
            transmitreceive,
            (std::uint8_t *const data_tx,
             std::uint8_t *data_rx,
             const std::uint8_t len),
            (const, override));
    };

    class MockAsyncTx : public async_tx_if
    {
    public:
        MOCK_METHOD(
            AsyncStartResult,
            start_transmit,
            (const std::uint8_t *source, std::size_t length),
            (override));

        MOCK_METHOD(bool, is_transmit_active, (), (const, override));
        MOCK_METHOD(bool, take_transmit_result, (AsyncResult & result), (override));
        MOCK_METHOD(bool, abort_transmit, (), (override));
    };

    template <std::size_t Size>
    class TransmitCoreProbe : public BusMasterTransmitCore<Size>
    {
    private:
        using Base = BusMasterTransmitCore<Size>;

    public:
        using EncodedFrame = typename Base::EncodedFrame;
        using Base::encode;
    };

    template <std::size_t MessageSize>
    class ReceiveCoreProbe : public BusMasterReceiveCore<MessageSize>
    {
    private:
        using Base = BusMasterReceiveCore<MessageSize>;

    public:
        using ReceiveArea = typename Base::ReceiveArea;
        using Base::append_received_data;
        using Base::buffered_size;
        using Base::check;
        using Base::commit_receive;
        using Base::prepare_receive_area;
        using Base::receive_area_prepared;
        using Base::rollback_receive;

        void seed(std::initializer_list<std::uint8_t> bytes)
        {
            this->frame_buffer_.clear();
            for (const auto byte : bytes)
            {
                this->frame_buffer_.push_back(byte);
            }
        }

        std::vector<std::uint8_t> contents() const
        {
            return {
                this->frame_buffer_.begin(),
                this->frame_buffer_.end()};
        }
    };

    void expect_simple_frame_encode(MockFramePack &frame)
    {
        EXPECT_CALL(
            frame,
            encode(MessageId::MotorCommand, _, 2U, _, _, _))
            .WillOnce(Invoke(
                [](const MessageId &,
                   const std::uint8_t *payload,
                   const std::size_t &payload_size,
                   std::uint8_t *output,
                   const std::int16_t &output_capacity,
                   std::size_t &output_size)
                {
                    EXPECT_EQ(payload_size, 2U);
                    EXPECT_GE(output_capacity, 3);

                    output[0] = 0xF7U;
                    std::copy(payload, payload + payload_size, output + 1U);
                    output_size = payload_size + 1U;
                    return Frame_NS::CommError::None;
                }));
    }

    TEST(BusMasterTransmitCore, SerializesAndReturnsViewOfEncodedFrame)
    {
        TransmitCoreProbe<16U> core;
        StrictMock<MockFramePack> frame;
        TestMessage message{0x1234U};
        TransmitCoreProbe<16U>::EncodedFrame encoded{};

        expect_simple_frame_encode(frame);

        EXPECT_EQ(core.encode(message, frame, encoded), Frame_NS::CommError::None);
        ASSERT_NE(encoded.data, nullptr);
        EXPECT_EQ(encoded.size, 3U);
        EXPECT_THAT(
            std::vector<std::uint8_t>(encoded.data, encoded.data + encoded.size),
            ElementsAre(0xF7U, 0x34U, 0x12U));
    }

    TEST(BusMasterTransmitCore, LeavesViewEmptyWhenFrameEncodingFails)
    {
        TransmitCoreProbe<16U> core;
        StrictMock<MockFramePack> frame;
        TestMessage message{0x1234U};
        TransmitCoreProbe<16U>::EncodedFrame encoded{
            reinterpret_cast<const std::uint8_t *>(0x1),
            99U};

        EXPECT_CALL(frame, encode(_, _, _, _, _, _))
            .WillOnce(Return(Frame_NS::CommError::BufferTooSmall));

        EXPECT_EQ(
            core.encode(message, frame, encoded),
            Frame_NS::CommError::BufferTooSmall);
        EXPECT_EQ(encoded.data, nullptr);
        EXPECT_EQ(encoded.size, 0U);
    }

    TEST(BusMasterTransmitCore, RejectsInvalidFrameLengthReturnedByEncoder)
    {
        TransmitCoreProbe<8U> core;
        StrictMock<MockFramePack> frame;
        TestMessage message{0x1234U};
        TransmitCoreProbe<8U>::EncodedFrame encoded{};

        EXPECT_CALL(frame, encode(_, _, _, _, _, _))
            .WillOnce(Invoke(
                [](const MessageId &,
                   const std::uint8_t *,
                   const std::size_t &,
                   std::uint8_t *,
                   const std::int16_t &,
                   std::size_t &output_size)
                {
                    output_size = 9U;
                    return Frame_NS::CommError::None;
                }));

        EXPECT_EQ(
            core.encode(message, frame, encoded),
            Frame_NS::CommError::InvalidLength);
        EXPECT_EQ(encoded.data, nullptr);
        EXPECT_EQ(encoded.size, 0U);
    }

    TEST(BusMasterTransmitSync, SendsEncodedFrame)
    {
        StrictMock<MockBus> bus;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitSync<16U> transmitter{bus};
        TestMessage message{0x1234U};

        expect_simple_frame_encode(frame);

        EXPECT_CALL(bus, transmit(_, 3U))
            .WillOnce(Invoke(
                [](const std::uint8_t *const data, const std::uint8_t len)
                {
                    EXPECT_EQ(len, 3U);
                    EXPECT_THAT(
                        std::vector<std::uint8_t>(data, data + len),
                        ElementsAre(0xF7U, 0x34U, 0x12U));
                    return 0;
                }));

        EXPECT_EQ(
            transmitter.transmit(message, frame),
            Frame_NS::CommError::None);
    }

    TEST(BusMasterTransmitSync, DoesNotUseBusWhenEncodingFails)
    {
        StrictMock<MockBus> bus;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitSync<16U> transmitter{bus};
        TestMessage message{};

        EXPECT_CALL(frame, encode(_, _, _, _, _, _))
            .WillOnce(Return(Frame_NS::CommError::CrcMismatch));

        EXPECT_EQ(
            transmitter.transmit(message, frame),
            Frame_NS::CommError::CrcMismatch);
    }

    TEST(BusMasterTransmitSync, MapsNonzeroTransportReturnToHardwareError)
    {
        StrictMock<MockBus> bus;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitSync<16U> transmitter{bus};
        TestMessage message{};

        expect_simple_frame_encode(frame);
        EXPECT_CALL(bus, transmit(_, 3U)).WillOnce(Return(-1));

        EXPECT_EQ(
            transmitter.transmit(message, frame),
            Frame_NS::CommError::HardwareError);
    }

    TEST(BusMasterTransmitAsync, StartsTransferAndRejectsSecondStartWhileActive)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{0x1234U};

        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, 3U))
            .WillOnce(Invoke(
                [](const std::uint8_t *data, std::size_t length)
                {
                    EXPECT_THAT(
                        std::vector<std::uint8_t>(data, data + length),
                        ElementsAre(0xF7U, 0x34U, 0x12U));
                    return AsyncStartResult::Started;
                }));

        EXPECT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);
        EXPECT_TRUE(transmitter.active());

        EXPECT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::Busy);
    }

    TEST(BusMasterTransmitAsync, MapsStartErrors)
    {
        TestMessage message{};

        {
            StrictMock<MockAsyncTx> async_tx;
            StrictMock<MockFramePack> frame;
            BusMasterTransmitAsync<16U> transmitter{async_tx};
            expect_simple_frame_encode(frame);
            EXPECT_CALL(async_tx, start_transmit(_, _))
                .WillOnce(Return(AsyncStartResult::Busy));
            EXPECT_EQ(
                transmitter.start_transmit(message, frame),
                Frame_NS::CommError::Busy);
        }

        {
            StrictMock<MockAsyncTx> async_tx;
            StrictMock<MockFramePack> frame;
            BusMasterTransmitAsync<16U> transmitter{async_tx};
            expect_simple_frame_encode(frame);
            EXPECT_CALL(async_tx, start_transmit(_, _))
                .WillOnce(Return(AsyncStartResult::InvalidArgument));
            EXPECT_EQ(
                transmitter.start_transmit(message, frame),
                Frame_NS::CommError::InvalidLength);
        }

        {
            StrictMock<MockAsyncTx> async_tx;
            StrictMock<MockFramePack> frame;
            BusMasterTransmitAsync<16U> transmitter{async_tx};
            expect_simple_frame_encode(frame);
            EXPECT_CALL(async_tx, start_transmit(_, _))
                .WillOnce(Return(AsyncStartResult::HardwareError));
            EXPECT_EQ(
                transmitter.start_transmit(message, frame),
                Frame_NS::CommError::HardwareError);
        }
    }

    TEST(BusMasterTransmitAsync, ReturnsBusyUntilCompletionIsAvailable)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, 3U))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);

        EXPECT_CALL(async_tx, take_transmit_result(_))
            .WillOnce(Return(false));

        EXPECT_EQ(
            transmitter.process_transmit(),
            Frame_NS::CommError::Busy);
        EXPECT_TRUE(transmitter.active());
    }

    TEST(BusMasterTransmitAsync, CompletesOnlyWhenAllBytesWereTransferred)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, 3U))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);

        EXPECT_CALL(async_tx, take_transmit_result(_))
            .WillOnce(Invoke(
                [](AsyncResult &result)
                {
                    result.event = AsyncEvent::Completed;
                    result.transferred_bytes = 3U;
                    return true;
                }));

        EXPECT_EQ(
            transmitter.process_transmit(),
            Frame_NS::CommError::None);
        EXPECT_FALSE(transmitter.active());
    }

    TEST(BusMasterTransmitAsync, RejectsPartialCompletion)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, 3U))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);

        EXPECT_CALL(async_tx, take_transmit_result(_))
            .WillOnce(Invoke(
                [](AsyncResult &result)
                {
                    result.event = AsyncEvent::Completed;
                    result.transferred_bytes = 2U;
                    return true;
                }));

        EXPECT_EQ(
            transmitter.process_transmit(),
            Frame_NS::CommError::HardwareError);
        EXPECT_FALSE(transmitter.active());
    }

    TEST(BusMasterTransmitAsync, MapsErrorAbortAndInvalidIdleToHardwareError)
    {
        for (const auto event : {
                 AsyncEvent::Error,
                 AsyncEvent::Aborted,
                 AsyncEvent::Idle,
                 AsyncEvent::None})
        {
            StrictMock<MockAsyncTx> async_tx;
            StrictMock<MockFramePack> frame;
            BusMasterTransmitAsync<16U> transmitter{async_tx};
            TestMessage message{};

            expect_simple_frame_encode(frame);
            EXPECT_CALL(async_tx, start_transmit(_, 3U))
                .WillOnce(Return(AsyncStartResult::Started));
            ASSERT_EQ(
                transmitter.start_transmit(message, frame),
                Frame_NS::CommError::None);

            EXPECT_CALL(async_tx, take_transmit_result(_))
                .WillOnce(Invoke(
                    [event](AsyncResult &result)
                    {
                        result.event = event;
                        return true;
                    }));

            EXPECT_EQ(
                transmitter.process_transmit(),
                Frame_NS::CommError::HardwareError);
            EXPECT_FALSE(transmitter.active());
        }
    }

    TEST(BusMasterTransmitAsync, AbortsActiveTransfer)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, 3U))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);

        EXPECT_CALL(async_tx, abort_transmit()).WillOnce(Return(true));
        EXPECT_TRUE(transmitter.abort_transmit());
        EXPECT_FALSE(transmitter.active());
    }

    TEST(BusMasterReceiveCore, PreparesAndCommitsDirectReceiveArea)
    {
        ReceiveCoreProbe<32U> core;

        const auto area = core.prepare_receive_area(10U);
        ASSERT_TRUE(static_cast<bool>(area));
        EXPECT_EQ(area.capacity, 10U);
        EXPECT_TRUE(core.receive_area_prepared());
        EXPECT_EQ(core.buffered_size(), 10U);

        area.data[0] = 0x11U;
        area.data[1] = 0x22U;
        area.data[2] = 0x33U;

        EXPECT_TRUE(core.commit_receive(3U));
        EXPECT_FALSE(core.receive_area_prepared());
        EXPECT_EQ(core.buffered_size(), 3U);
        EXPECT_THAT(core.contents(), ElementsAre(0x11U, 0x22U, 0x33U));
    }

    TEST(BusMasterReceiveCore, CapsReceiveAreaAtFixedFrameCapacity)
    {
        ReceiveCoreProbe<32U> core;

        const auto area = core.prepare_receive_area(1000U);
        ASSERT_TRUE(static_cast<bool>(area));
        EXPECT_EQ(area.capacity, 32U);
        core.rollback_receive();
    }

    TEST(BusMasterReceiveCore, RejectsSecondPreparationUntilCommitOrRollback)
    {
        ReceiveCoreProbe<32U> core;

        ASSERT_TRUE(static_cast<bool>(core.prepare_receive_area(10U)));
        EXPECT_FALSE(static_cast<bool>(core.prepare_receive_area(10U)));

        core.rollback_receive();
        EXPECT_TRUE(static_cast<bool>(core.prepare_receive_area(10U)));
        core.rollback_receive();
    }

    TEST(BusMasterReceiveCore, InvalidCommitRollsBackReservedRange)
    {
        ReceiveCoreProbe<32U> core;
        const std::array<std::uint8_t, 2U> existing{0xA1U, 0xA2U};
        ASSERT_TRUE(core.append_received_data(existing.data(), existing.size()));

        ASSERT_TRUE(static_cast<bool>(core.prepare_receive_area(5U)));
        EXPECT_FALSE(core.commit_receive(6U));

        EXPECT_FALSE(core.receive_area_prepared());
        EXPECT_THAT(core.contents(), ElementsAre(0xA1U, 0xA2U));
    }

    TEST(BusMasterReceiveCore, AppendPathSupportsExternallyOwnedTransportBuffers)
    {
        ReceiveCoreProbe<32U> core;
        const std::array<std::uint8_t, 4U> data{1U, 2U, 3U, 4U};

        EXPECT_TRUE(core.append_received_data(data.data(), data.size()));
        EXPECT_THAT(core.contents(), ElementsAre(1U, 2U, 3U, 4U));

        ASSERT_TRUE(static_cast<bool>(core.prepare_receive_area(4U)));
        EXPECT_FALSE(core.append_received_data(data.data(), data.size()));
        core.rollback_receive();
    }

    TEST(BusMasterReceiveCore, CheckRejectsEmptyBufferWithoutCallingDecoder)
    {
        ReceiveCoreProbe<32U> core;
        StrictMock<MockFrameUnpack> frame;
        MessageId id{};
        std::array<std::uint8_t, 32U> output{};
        std::size_t output_size = 99U;

        EXPECT_EQ(
            core.check(
                id,
                output.data(),
                static_cast<std::int16_t>(output.size()),
                output_size,
                frame),
            Frame_NS::CommError::InvalidLength);
        EXPECT_EQ(output_size, 0U);
    }

    TEST(BusMasterReceiveCore, CompleteFrameClearsBuffer)
    {
        ReceiveCoreProbe<32U> core;
        StrictMock<MockFrameUnpack> frame;
        MessageId id{};
        std::array<std::uint8_t, 32U> output{};
        std::size_t output_size = 0U;
        core.seed({0xF7U, 0x01U, 0x02U});

        EXPECT_CALL(frame, decode(_, _, 3U, _, _, _))
            .WillOnce(Invoke(
                [](MessageId &message_id,
                   const std::uint8_t *,
                   const std::size_t &,
                   std::uint8_t *output,
                   const std::int16_t &,
                   std::size_t &output_size)
                {
                    message_id = MessageId::MotorCommand;
                    output[0] = 0x42U;
                    output_size = 1U;
                    return Frame_NS::CommError::None;
                }));

        EXPECT_EQ(
            core.check(
                id,
                output.data(),
                static_cast<std::int16_t>(output.size()),
                output_size,
                frame),
            Frame_NS::CommError::None);
        EXPECT_EQ(core.buffered_size(), 0U);
        EXPECT_EQ(output_size, 1U);
        EXPECT_EQ(output[0], 0x42U);
    }

    TEST(BusMasterReceiveCore, UnfinishedFrameClearsSubmittedBytes)
    {
        ReceiveCoreProbe<32U> core;
        StrictMock<MockFrameUnpack> frame;
        MessageId id{};
        std::array<std::uint8_t, 32U> output{};
        std::size_t output_size = 0U;
        core.seed({0xF7U, 0x01U});

        EXPECT_CALL(frame, decode(_, _, 2U, _, _, _))
            .WillOnce(Return(Frame_NS::CommError::message_unfinished));

        EXPECT_EQ(
            core.check(
                id,
                output.data(),
                static_cast<std::int16_t>(output.size()),
                output_size,
                frame),
            Frame_NS::CommError::message_unfinished);
        EXPECT_EQ(core.buffered_size(), 0U);
    }

    TEST(BusMasterReceiveCore, FinishedFrameKeepsTrailingBytes)
    {
        ReceiveCoreProbe<32U> core;
        StrictMock<MockFrameUnpack> frame;
        MessageId id{};
        std::array<std::uint8_t, 32U> output{};
        std::size_t output_size = 0U;
        core.seed({0x10U, 0x11U, 0x12U, 0xAAU, 0xBBU});

        EXPECT_CALL(frame, decode(_, _, 5U, _, _, _))
            .WillOnce(Return(Frame_NS::CommError::message_finished_buffer_not_empty));
        EXPECT_CALL(frame, get_index()).WillOnce(Return(3U));

        EXPECT_EQ(
            core.check(
                id,
                output.data(),
                static_cast<std::int16_t>(output.size()),
                output_size,
                frame),
            Frame_NS::CommError::message_finished_buffer_not_empty);
        EXPECT_THAT(core.contents(), ElementsAre(0xAAU, 0xBBU));
    }

    TEST(BusMasterReceiveCore, InvalidConsumedIndexClearsBuffer)
    {
        for (const std::size_t invalid_index : {0U, 4U})
        {
            ReceiveCoreProbe<32U> core;
            StrictMock<MockFrameUnpack> frame;
            MessageId id{};
            std::array<std::uint8_t, 32U> output{};
            std::size_t output_size = 0U;
            core.seed({1U, 2U, 3U});

            EXPECT_CALL(frame, decode(_, _, 3U, _, _, _))
                .WillOnce(Return(Frame_NS::CommError::message_finished_buffer_not_empty));
            EXPECT_CALL(frame, get_index()).WillOnce(Return(invalid_index));

            EXPECT_EQ(
                core.check(
                    id,
                    output.data(),
                    static_cast<std::int16_t>(output.size()),
                    output_size,
                    frame),
                Frame_NS::CommError::InvalidLength);
            EXPECT_EQ(core.buffered_size(), 0U);
        }
    }

} // namespace
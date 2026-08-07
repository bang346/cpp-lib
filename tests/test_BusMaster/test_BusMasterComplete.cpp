#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "BusMasterReceiveAsync.hpp"
#include "BusMasterReceiveCore.hpp"
#include "BusMasterReceiveSync.hpp"
#include "BusMasterTransmitAsync.hpp"
#include "BusMasterTransmitCore.hpp"
#include "BusMasterTransmitSync.hpp"
#include "frame.hpp"

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

        void serialize(BinaryReader &reader)
        {
            reader(value);
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
             const Frame_NS::size_type &payload_size,
             std::uint8_t *output,
             const Frame_NS::size_type &output_capacity,
             Frame_NS::size_type &output_size),
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
             const Frame_NS::size_type &payload_size,
             std::uint8_t *output,
             const Frame_NS::size_type &output_capacity,
             Frame_NS::size_type &output_size),
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

    class MockAsyncRx : public async_rx_if
    {
    public:
        MOCK_METHOD(
            AsyncStartResult,
            start_receive,
            (std::uint8_t *destination, std::size_t capacity),
            (override));

        MOCK_METHOD(bool, is_receive_active, (), (const, override));
        MOCK_METHOD(bool, take_receive_result, (AsyncResult & result), (override));
        MOCK_METHOD(bool, abort_receive, (), (override));
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
    class ReceiveCoreProbe final : public BusMasterReceiveCore<MessageSize>
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
            return {this->frame_buffer_.begin(), this->frame_buffer_.end()};
        }
    };

    void expect_simple_frame_encode(MockFramePack &frame)
    {
        EXPECT_CALL(frame, encode(MessageId::MotorCommand, _, 2U, _, _, _))
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

    void expect_decode_bytes(
        MockFrameUnpack &frame,
        std::initializer_list<std::uint8_t> expected,
        Frame_NS::CommError result = Frame_NS::CommError::None)
    {
        const std::vector<std::uint8_t> expected_bytes(expected);
        EXPECT_CALL(frame, decode(_, _, expected_bytes.size(), _, _, _))
            .WillOnce(Invoke(
                [expected_bytes, result](
                    MessageId &message_id,
                    const std::uint8_t *payload,
                    const std::size_t &payload_size,
                    std::uint8_t *output,
                    const std::int16_t &,
                    std::size_t &output_size)
                {
                    EXPECT_EQ(
                        std::vector<std::uint8_t>(payload, payload + payload_size),
                        expected_bytes);
                    message_id = MessageId::MotorCommand;
                    output[0] = 0x34U;
                    output[1] = 0x12U;
                    output_size = 2U;
                    return result;
                }));
    }

    // -----------------------------------------------------------------------------
    // Transmit core
    // -----------------------------------------------------------------------------

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

    TEST(BusMasterTransmitCore, LeavesViewEmptyWhenEncodingFails)
    {
        TransmitCoreProbe<16U> core;
        StrictMock<MockFramePack> frame;
        TestMessage message{};
        TransmitCoreProbe<16U>::EncodedFrame encoded{
            reinterpret_cast<const std::uint8_t *>(0x1), 99U};

        EXPECT_CALL(frame, encode(_, _, _, _, _, _))
            .WillOnce(Return(Frame_NS::CommError::BufferTooSmall));

        EXPECT_EQ(
            core.encode(message, frame, encoded),
            Frame_NS::CommError::BufferTooSmall);
        EXPECT_EQ(encoded.data, nullptr);
        EXPECT_EQ(encoded.size, 0U);
    }

    TEST(BusMasterTransmitCore, RejectsImpossibleEncodedLength)
    {
        TransmitCoreProbe<8U> core;
        StrictMock<MockFramePack> frame;
        TestMessage message{};
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

    // -----------------------------------------------------------------------------
    // Synchronous transmit
    // -----------------------------------------------------------------------------

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
            .WillOnce(Return(Frame_NS::CommError::BufferTooSmall));

        EXPECT_EQ(
            transmitter.transmit(message, frame),
            Frame_NS::CommError::BufferTooSmall);
    }

    TEST(BusMasterTransmitSync, MapsTransportFailureToHardwareError)
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

    // -----------------------------------------------------------------------------
    // Asynchronous transmit
    // -----------------------------------------------------------------------------

    TEST(BusMasterTransmitAsync, StartsAndRejectsSecondStart)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{0x1234U};

        EXPECT_CALL(async_tx, is_transmit_active()).WillOnce(Return(false));
        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, 3U))
            .WillOnce(Return(AsyncStartResult::Started));

        EXPECT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);
        EXPECT_TRUE(transmitter.active());

        EXPECT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::Busy);
    }

    TEST(BusMasterTransmitAsync, ReturnsBusyUntilResultExists)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        EXPECT_CALL(async_tx, is_transmit_active()).WillOnce(Return(false));
        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, _))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);

        EXPECT_CALL(async_tx, take_transmit_result(_)).WillOnce(Return(false));
        EXPECT_EQ(
            transmitter.process_transmit(),
            Frame_NS::CommError::Busy);
        EXPECT_TRUE(transmitter.active());
    }

    TEST(BusMasterTransmitAsync, CompletesOnlyForExactByteCount)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        EXPECT_CALL(async_tx, is_transmit_active()).WillOnce(Return(false));
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

    TEST(BusMasterTransmitAsync, PartialCompletionIsHardwareError)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        EXPECT_CALL(async_tx, is_transmit_active()).WillOnce(Return(false));
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

    TEST(BusMasterTransmitAsync, AbortReleasesInternalBuffer)
    {
        StrictMock<MockAsyncTx> async_tx;
        StrictMock<MockFramePack> frame;
        BusMasterTransmitAsync<16U> transmitter{async_tx};
        TestMessage message{};

        EXPECT_CALL(async_tx, is_transmit_active()).WillOnce(Return(false));
        expect_simple_frame_encode(frame);
        EXPECT_CALL(async_tx, start_transmit(_, _))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(
            transmitter.start_transmit(message, frame),
            Frame_NS::CommError::None);

        EXPECT_CALL(async_tx, abort_transmit()).WillOnce(Return(true));
        EXPECT_TRUE(transmitter.abort_transmit());
        EXPECT_FALSE(transmitter.active());
    }

    // -----------------------------------------------------------------------------
    // Receive core
    // -----------------------------------------------------------------------------

    TEST(BusMasterReceiveCore, PreparesAndCommitsDirectArea)
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
        EXPECT_THAT(core.contents(), ElementsAre(0x11U, 0x22U, 0x33U));
    }

    TEST(BusMasterReceiveCore, RejectsSecondDirectArea)
    {
        ReceiveCoreProbe<32U> core;

        ASSERT_TRUE(static_cast<bool>(core.prepare_receive_area(10U)));
        EXPECT_FALSE(static_cast<bool>(core.prepare_receive_area(10U)));
        core.rollback_receive();
    }

    TEST(BusMasterReceiveCore, InvalidCommitRollsBackReservation)
    {
        ReceiveCoreProbe<32U> core;
        const std::array<std::uint8_t, 2U> existing{0xA1U, 0xA2U};
        ASSERT_TRUE(core.append_received_data(existing.data(), existing.size()));

        ASSERT_TRUE(static_cast<bool>(core.prepare_receive_area(5U)));
        EXPECT_FALSE(core.commit_receive(6U));

        EXPECT_FALSE(core.receive_area_prepared());
        EXPECT_THAT(core.contents(), ElementsAre(0xA1U, 0xA2U));
    }

    TEST(BusMasterReceiveCore, AppendPathIsBlockedDuringDirectReceive)
    {
        ReceiveCoreProbe<32U> core;
        const std::array<std::uint8_t, 3U> bytes{1U, 2U, 3U};

        EXPECT_TRUE(core.append_received_data(bytes.data(), bytes.size()));
        ASSERT_TRUE(static_cast<bool>(core.prepare_receive_area(4U)));
        EXPECT_FALSE(core.append_received_data(bytes.data(), bytes.size()));
        core.rollback_receive();
    }

    TEST(BusMasterReceiveCore, CompleteFrameClearsBuffer)
    {
        ReceiveCoreProbe<32U> core;
        StrictMock<MockFrameUnpack> frame;
        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        core.seed({0xF7U, 0x01U, 0x02U});
        expect_decode_bytes(frame, {0xF7U, 0x01U, 0x02U});

        EXPECT_EQ(
            core.check(id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::None);
        EXPECT_EQ(core.buffered_size(), 0U);
        EXPECT_EQ(output_size, 2U);
    }

    TEST(BusMasterReceiveCore, UnfinishedFrameClearsFreshInputBecauseDecoderOwnsState)
    {
        ReceiveCoreProbe<32U> core;
        StrictMock<MockFrameUnpack> frame;
        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        core.seed({0xF7U, 0x01U});
        expect_decode_bytes(
            frame,
            {0xF7U, 0x01U},
            Frame_NS::CommError::message_unfinished);

        EXPECT_EQ(
            core.check(id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::message_unfinished);
        EXPECT_EQ(core.buffered_size(), 0U);
    }

    TEST(BusMasterReceiveCore, FinishedFrameKeepsTrailingBytes)
    {
        ReceiveCoreProbe<32U> core;
        StrictMock<MockFrameUnpack> frame;
        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        core.seed({0x10U, 0x11U, 0x20U, 0x21U});
        expect_decode_bytes(
            frame,
            {0x10U, 0x11U, 0x20U, 0x21U},
            Frame_NS::CommError::message_finished_buffer_not_empty);
        EXPECT_CALL(frame, get_index()).WillOnce(Return(2U));

        EXPECT_EQ(
            core.check(id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::message_finished_buffer_not_empty);
        EXPECT_THAT(core.contents(), ElementsAre(0x20U, 0x21U));
    }

    // -----------------------------------------------------------------------------
    // Synchronous receive
    // -----------------------------------------------------------------------------

    TEST(BusMasterReceiveSync, ZeroTransportReturnMeansSuccessfulFullRead)
    {
        StrictMock<MockBus> bus;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveSync<32U> receiver{bus};

        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        EXPECT_CALL(bus, receive(_, 3U))
            .WillOnce(Invoke(
                [](std::uint8_t *const data, const std::uint8_t len)
                {
                    EXPECT_EQ(len, 3U);
                    data[0] = 0xF7U;
                    data[1] = 0x01U;
                    data[2] = 0x02U;
                    return 0; // bus_if success contract
                }));

        EXPECT_EQ(
            receiver.receive_raw(
                id, output.data(), output.size(), output_size, frame, 3U),
            Frame_NS::CommError::Timeout);
        EXPECT_EQ(output_size, 0U);
    }

    TEST(BusMasterReceiveSync, TransportErrorDoesNotCallDecoder)
    {
        StrictMock<MockBus> bus;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveSync<32U> receiver{bus};

        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        EXPECT_CALL(bus, receive(_, 3U)).WillOnce(Return(-1));

        EXPECT_EQ(
            receiver.receive_raw(
                id, output.data(), output.size(), output_size, frame, 3U),
            Frame_NS::CommError::HardwareError);
    }

    TEST(BusMasterReceiveSync, RejectsInvalidArgumentsBeforeUsingBus)
    {
        StrictMock<MockBus> bus;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveSync<32U> receiver{bus};

        MessageId id{};
        std::size_t output_size = 99U;

        EXPECT_EQ(
            receiver.receive_raw(id, nullptr, 10U, output_size, frame, 3U),
            Frame_NS::CommError::InvalidArgument);
    }

    // -----------------------------------------------------------------------------
    // Asynchronous receive
    // -----------------------------------------------------------------------------

    TEST(BusMasterReceiveAsync, StartPassesInternalFrameBufferToTransport)
    {
        StrictMock<MockAsyncRx> async_rx;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        EXPECT_CALL(async_rx, is_receive_active()).WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, 12U))
            .WillOnce(Invoke(
                [](std::uint8_t *destination, std::size_t capacity)
                {
                    EXPECT_NE(destination, nullptr);
                    EXPECT_EQ(capacity, 12U);
                    return AsyncStartResult::Started;
                }));

        EXPECT_EQ(receiver.start_receive(12U), Frame_NS::CommError::None);
        EXPECT_TRUE(receiver.active());
    }

    TEST(BusMasterReceiveAsync, SecondStartWhileActiveReturnsBusy)
    {
        StrictMock<MockAsyncRx> async_rx;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        EXPECT_CALL(async_rx, is_receive_active()).WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, _))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(receiver.start_receive(10U), Frame_NS::CommError::None);

        EXPECT_EQ(receiver.start_receive(10U), Frame_NS::CommError::Busy);
    }

    TEST(BusMasterReceiveAsync, StartFailureRollsBackReservation)
    {
        StrictMock<MockAsyncRx> async_rx;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        EXPECT_CALL(async_rx, is_receive_active())
            .WillOnce(Return(false))
            .WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, _))
            .WillOnce(Return(AsyncStartResult::HardwareError))
            .WillOnce(Return(AsyncStartResult::Started));

        EXPECT_EQ(
            receiver.start_receive(10U),
            Frame_NS::CommError::HardwareError);
        EXPECT_FALSE(receiver.active());

        // A second start must work, proving that rollback released the area.
        EXPECT_EQ(receiver.start_receive(10U), Frame_NS::CommError::None);
    }

    TEST(BusMasterReceiveAsync, ProcessReturnsBusyUntilResultIsAvailable)
    {
        StrictMock<MockAsyncRx> async_rx;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        EXPECT_CALL(async_rx, is_receive_active()).WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, _))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(receiver.start_receive(10U), Frame_NS::CommError::None);

        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        EXPECT_CALL(async_rx, take_receive_result(_)).WillOnce(Return(false));
        EXPECT_EQ(
            receiver.process_receive(
                id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::Busy);
        EXPECT_TRUE(receiver.active());
    }

    TEST(BusMasterReceiveAsync, CompletedEventCommitsAndDecodesReceivedBytes)
    {
        StrictMock<MockAsyncRx> async_rx;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        std::uint8_t *dma_destination = nullptr;

        EXPECT_CALL(async_rx, is_receive_active()).WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, 10U))
            .WillOnce(Invoke(
                [&dma_destination](std::uint8_t *destination, std::size_t)
                {
                    dma_destination = destination;
                    return AsyncStartResult::Started;
                }));
        ASSERT_EQ(receiver.start_receive(10U), Frame_NS::CommError::None);
        ASSERT_NE(dma_destination, nullptr);

        dma_destination[0] = 0xF7U;
        dma_destination[1] = 0x01U;
        dma_destination[2] = 0x02U;

        EXPECT_CALL(async_rx, take_receive_result(_))
            .WillOnce(Invoke(
                [](AsyncResult &result)
                {
                    result.event = AsyncEvent::Completed;
                    result.transferred_bytes = 3U;
                    return true;
                }));
        expect_decode_bytes(frame, {0xF7U, 0x01U, 0x02U});

        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        EXPECT_EQ(
            receiver.process_receive(
                id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::None);
        EXPECT_FALSE(receiver.active());
        EXPECT_EQ(output_size, 2U);
    }

    TEST(BusMasterReceiveAsync, IdleEventIsValidReceiveCompletion)
    {
        StrictMock<MockAsyncRx> async_rx;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        std::uint8_t *dma_destination = nullptr;
        EXPECT_CALL(async_rx, is_receive_active()).WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, _))
            .WillOnce(Invoke(
                [&dma_destination](std::uint8_t *destination, std::size_t)
                {
                    dma_destination = destination;
                    return AsyncStartResult::Started;
                }));
        ASSERT_EQ(receiver.start_receive(10U), Frame_NS::CommError::None);

        dma_destination[0] = 0xF7U;
        dma_destination[1] = 0x01U;

        EXPECT_CALL(async_rx, take_receive_result(_))
            .WillOnce(Invoke(
                [](AsyncResult &result)
                {
                    result.event = AsyncEvent::Idle;
                    result.transferred_bytes = 2U;
                    return true;
                }));
        expect_decode_bytes(frame, {0xF7U, 0x01U});

        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        EXPECT_EQ(
            receiver.process_receive(
                id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::None);
    }

    TEST(BusMasterReceiveAsync, OversizedCompletionIsRejectedAndReservationReleased)
    {
        StrictMock<MockAsyncRx> async_rx;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        EXPECT_CALL(async_rx, is_receive_active())
            .WillOnce(Return(false))
            .WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, 5U))
            .WillOnce(Return(AsyncStartResult::Started))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(receiver.start_receive(5U), Frame_NS::CommError::None);

        EXPECT_CALL(async_rx, take_receive_result(_))
            .WillOnce(Invoke(
                [](AsyncResult &result)
                {
                    result.event = AsyncEvent::Completed;
                    result.transferred_bytes = 6U;
                    return true;
                }));

        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        EXPECT_EQ(
            receiver.process_receive(
                id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::ClassInternalBufferTooSmall);
        EXPECT_FALSE(receiver.active());

        EXPECT_EQ(receiver.start_receive(5U), Frame_NS::CommError::None);
    }

    TEST(BusMasterReceiveAsync, ErrorRollsBackAndReleasesBuffer)
    {
        StrictMock<MockAsyncRx> async_rx;
        StrictMock<MockFrameUnpack> frame;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        EXPECT_CALL(async_rx, is_receive_active())
            .WillOnce(Return(false))
            .WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, _))
            .WillOnce(Return(AsyncStartResult::Started))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(receiver.start_receive(8U), Frame_NS::CommError::None);

        EXPECT_CALL(async_rx, take_receive_result(_))
            .WillOnce(Invoke(
                [](AsyncResult &result)
                {
                    result.event = AsyncEvent::Error;
                    result.hardware_error = 123U;
                    return true;
                }));

        MessageId id{};
        std::array<std::uint8_t, 16U> output{};
        std::size_t output_size = 0U;

        EXPECT_EQ(
            receiver.process_receive(
                id, output.data(), output.size(), output_size, frame),
            Frame_NS::CommError::HardwareError);
        EXPECT_FALSE(receiver.active());

        EXPECT_EQ(receiver.start_receive(8U), Frame_NS::CommError::None);
    }

    TEST(BusMasterReceiveAsync, AbortRollsBackDirectReceiveArea)
    {
        StrictMock<MockAsyncRx> async_rx;
        BusMasterReceiveAsync<32U> receiver{async_rx};

        EXPECT_CALL(async_rx, is_receive_active())
            .WillOnce(Return(false))
            .WillOnce(Return(false));
        EXPECT_CALL(async_rx, start_receive(_, _))
            .WillOnce(Return(AsyncStartResult::Started))
            .WillOnce(Return(AsyncStartResult::Started));
        ASSERT_EQ(receiver.start_receive(8U), Frame_NS::CommError::None);

        EXPECT_CALL(async_rx, abort_receive()).WillOnce(Return(true));
        EXPECT_TRUE(receiver.abort_receive());
        EXPECT_FALSE(receiver.active());

        EXPECT_EQ(receiver.start_receive(8U), Frame_NS::CommError::None);
    }

} // namespace
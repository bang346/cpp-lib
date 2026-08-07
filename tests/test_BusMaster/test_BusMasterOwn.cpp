#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <vector>
#include <thread>
#include <chrono>
#include <future>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "frameV1.hpp"

#include "BusMasterReceiveCore.hpp"
#include "BusMasterTransmitAsync.hpp"
#include "BusMasterTransmitCore.hpp"
#include "BusMasterTransmitSync.hpp"

#include "mock_bus.hpp"
using ::testing::_;
using ::testing::Invoke;
using ::testing::StrictMock;

using namespace std::chrono_literals;
struct TestMessage
{
    std::uint16_t value{0x1234};

    void serialize(BinaryWriter &writer)
    {
        writer(value);
    }
};

template <>
struct MessageTraits<TestMessage>
{
    static constexpr MessageId id = MessageId::MotorCommand;
    static constexpr std::size_t maximumSize =
        sizeof(std::uint16_t);
};

class BusMasterTransmitAsyncTest : public ::testing::Test
{
protected:
    StrictMock<mock_bus> bus;

    crc_wrapper<std::uint16_t> crc{
        0xFFFFu,
        0x1021u};

    // KEIN Template-Parameter!
    frameV1_pack<std::uint16_t> frame{crc};

    BusMasterTransmitAsync<128> master{bus};

    std::thread worker;

    void TearDown() override
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
};

TEST_F(BusMasterTransmitAsyncTest, TransmitCompletesAsynchronously)
{
    auto time = 10ms;
    std::atomic<bool> transmission_finished{false};

    std::promise<void> transmission_started;
    auto started_future =
        transmission_started.get_future();

    std::size_t transmitted_length = 0;

    // ---------------------------------------------------------
    // start_transmit()
    // ---------------------------------------------------------

    EXPECT_CALL(bus, start_transmit(_, _))
        .WillOnce(
            Invoke(
                [&](const std::uint8_t *data,
                    std::size_t length)
                {
                    EXPECT_NE(data, nullptr);
                    EXPECT_GT(length, 0u);

                    transmitted_length = length;

                    // FrameV1 Startbyte
                    EXPECT_EQ(
                        data[0],
                        0xF7);

                    transmission_started.set_value();

                    return AsyncStartResult::Started;
                }));

    // ---------------------------------------------------------
    // take_transmit_result()
    // ---------------------------------------------------------

    EXPECT_CALL(bus, take_transmit_result(_))
        .WillRepeatedly(
            Invoke(
                [&](AsyncResult &result)
                {
                    if (!transmission_finished.load())
                    {
                        return false;
                    }

                    result.event =
                        AsyncEvent::Completed;

                    result.transferred_bytes =
                        transmitted_length;

                    return true;
                }));

    // ---------------------------------------------------------
    // Fake Hardware Thread
    // ---------------------------------------------------------

    worker = std::thread(
        [&]()
        {
            started_future.wait();

            std::this_thread::sleep_for(time);

            transmission_finished.store(true);
        });

    // ---------------------------------------------------------
    // Test
    // ---------------------------------------------------------

    TestMessage message{
        0x1234};

    EXPECT_EQ(master.start_transmit(message, frame), Frame_NS::CommError::None);

    EXPECT_TRUE(master.active());

    const auto start = std::chrono::steady_clock::now();
    // DMA Simulation vollständig beenden
    while (master.process_transmit() != Frame_NS::CommError::None)
    {
    }
    const auto end = std::chrono::steady_clock::now();

    auto expected_time = std::chrono::duration<double>(time).count();
    auto real_time = std::chrono::duration<double>(end - start).count();
    int i = 0;
    EXPECT_NEAR(expected_time, real_time, 0.01);

    // ---------------------------------------------------------
    // Ergebnis vom Bus abholen
    // ---------------------------------------------------------

    EXPECT_FALSE(master.active());
}
#include <gtest/gtest.h>
#include <string>
#include <stdio.h>
#include <chrono>

#include "BusMasterReceiveSync.hpp"
#include "BusMasterTransmitSync.hpp"
#include "rs232_w.hpp"
#include "archive_generic.hpp"
#include "frameV1.hpp"
#include "prittyprinting.hpp"

// TEST(RS232, Send)
// {
//     GTEST_SKIP();
//     std::string port = "COM5";
//     windows_uart DUT(port);

//     BusMaster_NS::BusMasterTransmit<64> bus;
//     crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
//     frameV1_pack frame_pack(coder);

//     ConfigureGeneric<9> message =
//         {
//             255,
//             {1, 2, 3, 4, 5, 6, 7, 8, 9}};
//     bus.transmit(message, &DUT, frame_pack);
// }

// TEST(RS232, Receive)
// {
//     GTEST_SKIP();
//     std::string port = "COM5";
//     windows_uart DUT(port);

//     BusMaster_NS::BusMasterReceive<1000> bus;
//     crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
//     frameV1_unpack frame_unpack(coder);

//     ConfigureGeneric<12> message{};
//     int counter = 0;
//     while (true)
//     {
//         auto result = bus.receive_message(message, &DUT, frame_unpack);
//         if (result == Frame_NS::CommError::None || result == Frame_NS::CommError::message_finished_buffer_not_empty)
//         {
//             counter++;

//             std::cout << GREEN << " Command: " << (int)message.command << "\n";
//             for (size_t i = 0; i < 12; i++)
//             {
//                 std::cout << "byte[" << i << "]: " << (int)message.Payload[i] << " | ";
//             }
//             std::cout << RESET << std::endl;
//         }
//     }
// }

TEST(RS232, TransmitReceive)
{
    constexpr std::size_t size = 12;
    constexpr std::size_t calls = 1000;
    // Transmit Part
    std::string port = "COM5";
    windows_uart DUT(port, 115200);

    BusMasterReceiveSync<1000> receiver(DUT);
    BusMasterTransmitSync<1000> transmitter(DUT);

    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    frameV1_pack frame_pack(coder);
    frameV1_unpack frame_unpack(coder);
    int tests = 0;
    ConfigureGeneric<size> message_tx{};
    for (size_t i = 0; i < size; i++)
    {
        message_tx.Payload[i] = i;
    }

    const auto start = std::chrono::steady_clock::now();
    while (tests++ < calls)
    {
        message_tx.command = 255;
        transmitter.transmit(message_tx, frame_pack);

        ConfigureGeneric<size> message{};
        int counter = 0;
        int trys = 0;
        const auto start = std::chrono::steady_clock::now();
        while (receiver.receive_message(message, frame_unpack) != Frame_NS::CommError::None)
        {
            if (counter++ > 0)
            {
                FAIL();
            }
        }
        // const auto end = std::chrono::steady_clock::now();
        // auto duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        // std::cout << "Zeit: " << duration_ms.count() << " us\n";
        // std::cout << GREEN << " Command: " << (int)message.command << "\n";
        // for (size_t i = 0; i < 12; i++)
        // {
        //     std::cout << "byte[" << i << "]: " << (int)message.Payload[i] << " | ";
        // }
        // std::cout << RESET << std::endl;
    }
    const auto end = std::chrono::steady_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    const double datarate = ((double)(2 * 1000 * calls * (frame_unpack.get_MaxSize() + MessageTraits<ConfigureGeneric<size>>::maximumSize + 2))) / ((double)(duration_ms.count()));
    const double maxdatarate = 115200.0 / 10.0;
    std::cout << "Time for " << calls << " requests: " << duration_ms.count() << " ms\n";
    std::cout << YELLOW << "Datarate: " << datarate << "byte/s" << RESET << std::endl;
    std::cout << YELLOW << "Datarate: " << datarate / maxdatarate * 100.0 << "[%]" << RESET << std::endl;
}
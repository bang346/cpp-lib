#include <gtest/gtest.h>
#include <string>
#include <stdio.h>

#include "BusMaster.hpp"
#include "rs232_w.hpp"
#include "archive_generic.hpp"
#include "frameV1.hpp"
#include "prittyprinting.hpp"

TEST(RS232, Send)
{
    GTEST_SKIP();
    std::string port = "COM5";
    windows_uart DUT(port);

    BusMaster_NS::BusMasterTransmit<64> bus;
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    frameV1_pack frame_pack(coder);

    ConfigureGeneric<9> message =
        {
            255,
            {1, 2, 3, 4, 5, 6, 7, 8, 9}};
    bus.transmit(message, &DUT, frame_pack);
}

TEST(RS232, Receive)
{
    GTEST_SKIP();
    std::string port = "COM5";
    windows_uart DUT(port);

    BusMaster_NS::BusMasterReceive<1000> bus;
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    frameV1_unpack frame_unpack(coder);

    ConfigureGeneric<12> message{};
    int counter = 0;
    while (true)
    {
        auto result = bus.receive_message(message, &DUT, frame_unpack);
        if (result == Frame_NS::CommError::None || result == Frame_NS::CommError::message_finished_buffer_not_empty)
        {
            counter++;

            std::cout << GREEN << " Command: " << (int)message.command << "\n";
            for (size_t i = 0; i < 12; i++)
            {
                std::cout << "byte[" << i << "]: " << (int)message.Payload[i] << " | ";
            }
            std::cout << RESET << std::endl;
        }
    }
}

TEST(RS232, TransmitReceive)
{
    // Transmit Part
    std::string port = "COM5";
    windows_uart DUT(port);

    BusMaster_NS::BusMasterTransmit<64> bus_tx;
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);
    frameV1_pack frame_pack(coder);
    int tests = 0;
    ConfigureGeneric<12> message_tx =
        {
            255,
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}};
    while (tests++ < 100)
    {
        bus_tx.transmit(message_tx, &DUT, frame_pack);

        // Receive Part
        BusMaster_NS::BusMasterReceive<1000> bus;
        frameV1_unpack frame_unpack(coder);

        ConfigureGeneric<12> message{};
        int counter = 0;
        int trys = 0;

        while (bus.receive_message(message, &DUT, frame_unpack) != Frame_NS::CommError::None)
        {
            if (counter++ > 100)
            {
                FAIL();
            }
        }

        std::cout << GREEN << " Command: " << (int)message.command << "\n";
        for (size_t i = 0; i < 12; i++)
        {
            std::cout << "byte[" << i << "]: " << (int)message.Payload[i] << " | ";
        }
        std::cout << RESET << std::endl;
    }
}
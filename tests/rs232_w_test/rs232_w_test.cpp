#include <gtest/gtest.h>
#include <string>

#include "BusMaster.hpp"
#include "rs232_w.hpp"
#include "archive_generic.hpp"
#include "frameV1.hpp"

TEST(RS232, Send)
{
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
#include <gtest/gtest.h>
#include <cstdint>
#include <cstddef>

// --- Include an dein Layout anpassen ---
// Wenn Header liegt in Core/Math/crc/include/crc.hpp:
#include "crc.hpp"

// Wenn Header liegt in Core/Math/crc/include/mylib/crc.hpp, dann stattdessen:
// #include "mylib/crc.hpp"

static constexpr uint8_t kTestVec_123456789[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9'};

TEST(CRC, CRC16_CCITT_FALSE_123456789)
{
    // CRC-16/CCITT-FALSE:
    // width=16, poly=0x1021, init=0xFFFF, refin=false, refout=false, xorout=0x0000
    const uint16_t crc16 = crc_compute<uint16_t>(
        kTestVec_123456789,
        sizeof(kTestVec_123456789),
        0xFFFFu,
        0x1021u,
        CrcBitOrder::MsbFirst,
        0x0000u);

    EXPECT_EQ(crc16, 0x29B1u);
}

TEST(CRC, CRC32_Ethernet_123456789)
{
    // CRC-32 (Ethernet/ISO-HDLC):
    // width=32, poly(reflected)=0xEDB88320, init=0xFFFFFFFF, refin/refout=true, xorout=0xFFFFFFFF
    const uint32_t crc32 = crc_compute<uint32_t>(
        kTestVec_123456789,
        sizeof(kTestVec_123456789),
        0xFFFFFFFFu,
        0xEDB88320u,
        CrcBitOrder::LsbFirst,
        0xFFFFFFFFu);

    EXPECT_EQ(crc32, 0xCBF43926u);
}

TEST(CRC, EmptyBuffer_ReturnsInitXorOut)
{
    // Bei len=0: deine Funktion gibt init ^ xor_out zurück
    const uint16_t v = crc_compute<uint16_t>(
        nullptr,
        0,
        0x1234u,
        0x1021u,
        CrcBitOrder::MsbFirst,
        0xFFFFu);

    EXPECT_EQ(v, static_cast<uint16_t>(0x1234u ^ 0xFFFFu));
}

TEST(CRC, NullDataWithNonzeroLen_IsHandled)
{
    // Wenn du data==nullptr und len>0 zulassen willst, würdest du hier ggf. anders entscheiden.
    // In deiner Implementation: data==nullptr -> init^xor_out
    const uint32_t v = crc_compute<uint32_t>(
        nullptr,
        10,
        0x11111111u,
        0xEDB88320u,
        CrcBitOrder::LsbFirst,
        0x22222222u);

    EXPECT_EQ(v, (0x11111111u ^ 0x22222222u));
}

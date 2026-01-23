#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <limits>

enum class CrcBitOrder : uint8_t
{
    MsbFirst, // "normal" (shift left, test MSB)  -> poly z.B. 0x1021, 0x04C11DB7
    LsbFirst  // "reflected" (shift right, test LSB) -> poly z.B. 0xA001, 0xEDB88320
};

template <typename CrcT>
constexpr CrcT crc_compute(const uint8_t *data,
                           std::size_t len,
                           CrcT init,
                           CrcT poly,
                           CrcBitOrder order = CrcBitOrder::MsbFirst,
                           CrcT xor_out = static_cast<CrcT>(0))
{
    static_assert(std::is_unsigned<CrcT>::value, "CrcT must be an unsigned integer type.");

    constexpr unsigned W = std::numeric_limits<CrcT>::digits; // 16 for uint16_t, 32 for uint32_t, ...
    static_assert(W >= 8, "CRC width must be >= 8 bits for this implementation.");

    const CrcT topbit = static_cast<CrcT>(CrcT(1) << (W - 1));
    CrcT crc = init;

    if (data == nullptr || len == 0)
    {
        return static_cast<CrcT>(crc ^ xor_out);
    }

    if (order == CrcBitOrder::MsbFirst)
    {
        // MSB-first: XOR Byte in die oberen 8 Bits, dann 8x shiften (left)
        for (std::size_t i = 0; i < len; ++i)
        {
            crc ^= static_cast<CrcT>(static_cast<CrcT>(data[i]) << (W - 8));
            for (unsigned b = 0; b < 8; ++b)
            {
                crc = (crc & topbit) ? static_cast<CrcT>((crc << 1) ^ poly)
                                     : static_cast<CrcT>(crc << 1);
            }
        }
    }
    else
    {
        // LSB-first (reflected): XOR Byte in die unteren 8 Bits, dann 8x shiften (right)
        for (std::size_t i = 0; i < len; ++i)
        {
            crc ^= static_cast<CrcT>(data[i]);
            for (unsigned b = 0; b < 8; ++b)
            {
                crc = (crc & static_cast<CrcT>(1)) ? static_cast<CrcT>((crc >> 1) ^ poly)
                                                   : static_cast<CrcT>(crc >> 1);
            }
        }
    }

    return static_cast<CrcT>(crc ^ xor_out);
}
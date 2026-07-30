#ifndef CODER_HPP
#define CODER_HPP

#include <cstdint>

#include "coder_if.hpp"
#include "crc.hpp"

template <typename CrcT>
class coder_crc : public coder_if
{
private:
    const CrcT init_;
    const CrcT poly_;
    const CrcBitOrder order_;
    const CrcT xor_out_;

    bool decode_result_;

public:
    /// @brief Constructor
    /// @param init         Inital value for crc-calculation
    /// @param poly         Polinominal
    /// @param order        Bitorder
    /// @param xor_out      Xor out
    coder_crc(const CrcT &init, const CrcT &poly, const CrcBitOrder &order = CrcBitOrder::MsbFirst, CrcT xor_out = static_cast<CrcT>(0))
        : init_{init},
          poly_{poly},
          order_{order},
          xor_out_{xor_out},
          decode_result_{false}
    {
    }

    /// @brief Interface Method to code a message
    /// @param data         Data wich will coded
    /// @param len          len of the data
    /// @param result       Coded data
    /// @return             Len of the new data
    virtual coder_if_NS::len_t code(const std::uint8_t *data, const coder_if_NS::len_t &len, std::uint8_t *result) const override
    {
        if (!data)
        {
            return 0;
        }
        const CrcT crc = crc_compute<CrcT>(data, len, init_, poly_, order_, xor_out_);

        for (std::size_t i = 0; i < len; i++)
        {
            result[i] = data[i];
        }

        using UnsignedT = typename std::make_unsigned<CrcT>::type;

        const UnsignedT raw =
            static_cast<UnsignedT>(crc);

        // Little Endian
        for (std::size_t i = 0; i < sizeof(CrcT); i++)
        {
            result[len + i] =
                static_cast<std::uint8_t>(
                    (raw >> (8U * i)) & 0xFFU);
        }
        return len + sizeof(CrcT);
    }

    /// @brief Interface Method to decode a message
    /// @param data         Data wich will decoded
    /// @param len          len of the data
    /// @param result       Coded data
    /// @return             Len of the new data
    virtual coder_if_NS::len_t decode(const std::uint8_t *data, const coder_if_NS::len_t &len, std::uint8_t *result) override
    {
        for (size_t i = 0; i < len - sizeof(CrcT); i++)
        {
            result[i] = data[i];
        }
        const CrcT crc_should = crc_compute<CrcT>(data, len - sizeof(CrcT), init_, poly_, order_, xor_out_);
        CrcT crc_is = 0;

        using UnsignedT = typename std::make_unsigned<CrcT>::type;

        UnsignedT raw = 0;

        // Little Endian
        for (std::size_t i = 0; i < sizeof(CrcT); i++)
        {
            raw |= static_cast<UnsignedT>(
                       data[i + len - sizeof(CrcT)])
                   << (8U * i);
        }

        crc_is = static_cast<CrcT>(raw);

        decode_result_ = (crc_is == crc_should);
        return len - sizeof(CrcT);
    }

    virtual bool decode_result() const override
    {

        return decode_result_;
    }
};

template <typename CrcT>
class crc_wrapper
{
private:
    const CrcT init_;
    const CrcT poly_;
    const CrcBitOrder order_;
    const CrcT xor_out_;

    bool decode_result_;

public:
    /// @brief Constructor
    /// @param init         Inital value for crc-calculation
    /// @param poly         Polinominal
    /// @param order        Bitorder
    /// @param xor_out      Xor out
    crc_wrapper(const CrcT &init, const CrcT &poly, const CrcBitOrder &order = CrcBitOrder::MsbFirst, CrcT xor_out = static_cast<CrcT>(0))
        : init_{init},
          poly_{poly},
          order_{order},
          xor_out_{xor_out},
          decode_result_{false}
    {
    }

    /// @brief Interface Method to code a message
    /// @param data         Data wich will coded
    /// @param len          len of the data
    /// @param result       Coded data
    /// @return             Len of the new data
    virtual CrcT code(const std::uint8_t *data, const coder_if_NS::len_t &len) const
    {
        if (!data)
        {
            return 0;
        }
        return crc_compute<CrcT>(data, len, init_, poly_, order_, xor_out_);
    };

    /// @brief Interface Method to code a message
    /// @param data         Data wich will coded
    /// @param len          len of the data
    /// @param result       Coded data
    /// @return             Len of the new data
    virtual CrcT update(const std::uint8_t *data, const CrcT &init, const coder_if_NS::len_t &len) const
    {
        if (!data)
        {
            return 0;
        }
        return crc_compute<CrcT>(data, len, init, poly_, order_, xor_out_);
    };
};

#endif

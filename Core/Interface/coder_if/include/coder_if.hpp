#ifndef CODER_IF_HPP
#define CODER_IF_HPP

#include <cstdint>

namespace coder_if_NS
{

    typedef std::uint16_t len_t;
} // namespace coder_if_NS

struct coder_if
{
    /// @brief Interface Method to code a message
    /// @param data         Data wich will coded
    /// @param len          len of the data
    /// @param result       Coded data
    /// @param capacity     Capacity of the result buffer
    /// @return             Len of the new data
    virtual coder_if_NS::len_t code(const std::uint8_t *data, const coder_if_NS::len_t &len, std::uint8_t *result, coder_if_NS::len_t capacity) const = 0;

    /// @brief Interface Method to decode a message
    /// @param data         Data wich will decoded
    /// @param len          len of the data
    /// @param result       Coded data
    /// @param capacity     Capacity of the result buffer
    /// @return             Len of the new data
    virtual coder_if_NS::len_t decode(const std::uint8_t *data, const coder_if_NS::len_t &len, std::uint8_t *result, coder_if_NS::len_t capacity) = 0;

    /// @brief Returns the result of the decoding
    /// @return             true = success,
    ///                     false = error
    virtual bool decode_result() const = 0;
};

#endif

#ifndef BINARY_RW_HPP
#define BINARY_RW_HPP

#include <array>

/// @brief Class to serialize data
/// @note           Serialization is little endian
class BinaryWriter
{
public:
    /// @brief Class constructor
    /// @param buffer       Pointer to the external buffer
    /// @param capacity     Capacity of the buffer
    BinaryWriter(std::uint8_t *buffer, std::size_t capacity)
        : buffer_(buffer),
          capacity_(capacity)
    {
    }

    /// @brief Overloaded class operator
    /// @details            Allows the defined structs to have an
    ///                     undefined number of parameters, all of
    ///                     which are then captured and stored
    ///                     using the ellipsis operator.
    /// @note               The method serialize must be defined in
    ///                     desired struct!
    /// @tparam ...Types    Different types wich will be serialized
    /// @param ...values    Different values wich will be serialized
    /// @return             true = success,
    ///                     false = error
    template <typename... Types>
    bool operator()(Types &...values)
    {
        return (write(values) && ...);
    }

    template <typename T, std::size_t N>
    bool write(const std::array<T, N> &values)
    {
        for (const auto &value : values)
        {
            if (!write(value))
            {
                return false;
            }
        }
        return true;
    }

    /// @brief Method wich will save the serialized data inside the class
    /// @tparam T           Datatype of the to serialize data
    /// @param value        Value
    /// @return             true = success,
    ///                     false = error
    template <typename T>
    bool write(T value)
    {
        static_assert(
            std::is_integral<T>::value,
            "BinaryWriter unterstützt nur Integer-Typen");

        if (position_ + sizeof(T) > capacity_) // leave if no room is left
        {
            return false;
        }

        using UnsignedT = typename std::make_unsigned<T>::type;

        const UnsignedT raw =
            static_cast<UnsignedT>(value);

        // Little Endian
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            buffer_[position_++] =
                static_cast<std::uint8_t>(
                    (raw >> (8U * i)) & 0xFFU);
        }

        return true;
    }

    std::size_t size() const
    {
        return position_;
    }

private:
    std::uint8_t *buffer_;
    std::size_t capacity_;
    std::size_t position_ = 0;
};

// ============================================================
// BinaryReader
// ============================================================

/// @brief Class to deserialize data
/// @note           Deserialization is little endian
class BinaryReader
{
public:
    BinaryReader(
        const std::uint8_t *buffer,
        std::size_t size)
        : buffer_(buffer),
          size_(size)
    {
    }

    /// @brief Overloaded class operator
    /// @details            Allows the defined structs to have an
    ///                     undefined number of parameters, all of
    ///                     which are then captured and stored
    ///                     using the ellipsis operator.
    /// @note               The method serialize must be defined in
    ///                     desired struct!
    /// @tparam ...Types    Different types wich will be serialized
    /// @param ...values    Different values wich will be serialized
    /// @return             true = success,
    ///                     false = error
    template <typename... Types>
    bool operator()(Types &...values)
    {
        return (read(values) && ...);
    }

    /// @brief Method wich will save the deserialized data inside the class
    /// @tparam T           Datatype of the to serialize data
    /// @param value        Value
    /// @return             true = success,
    ///                     false = error
    template <typename T>
    bool read(T &value)
    {
        static_assert(
            std::is_integral<T>::value,
            "BinaryReader unterstützt nur Integer-Typen");

        if (position_ + sizeof(T) > size_)
        {
            return false;
        }

        using UnsignedT = typename std::make_unsigned<T>::type;

        UnsignedT raw = 0;

        // Little Endian
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            raw |= static_cast<UnsignedT>(
                       buffer_[position_++])
                   << (8U * i);
        }

        value = static_cast<T>(raw);

        return true;
    }

    template <typename T, std::size_t N>
    bool read(std::array<T, N> &values)
    {
        for (auto &value : values)
        {
            if (!read(value))
            {
                return false;
            }
        }

        return true;
    }

    bool finished() const
    {
        return position_ == size_;
    }

    inline std::size_t size() const { return position_; }

private:
    const std::uint8_t *buffer_;
    std::size_t size_;
    std::size_t position_ = 0;
};

#endif
#pragma once
#include <cstddef>

#include "frame.hpp"
#include "binary_container.hpp"
#include "etl/vector.h"

/// @brief Receive core class
/// @note               This class is used to abstract the
///                     different receive methods (DMA, polling, circular(future))
/// @tparam MessageSize Internal buffer size to create the frame when all data is received
template <std::size_t MessageSize>
class BusMasterReceiveCore
{
    static_assert(MessageSize > 0U, "BusMasterReceiveCore requires a non-zero message buffer");

protected:
    // std::array<std::uint8_t, MessageSize> message_buffer_{};

    // Die feste 100 bleibt bei dir bestehen.
    etl::vector<std::uint8_t, MessageSize> frame_buffer_{}; // Receive buffer before frame creation

    // Helper struct for frame_buffer
    struct ReceiveArea
    {
        std::uint8_t *data{nullptr};                          // Position of the newest valid data inside the vector
        std::size_t capacity{0U};                             // Remaining capacity
        [[nodiscard]] explicit operator bool() const noexcept // Shows if the Received data is valid
        {
            return data != nullptr && capacity > 0U;
        }
    };

    /// @brief Method to make room inside the receive vector
    /// @note                   Used to increase the size of the etl
    ///                         vector, before a receive call. As a result of that it is
    ///                         possible to index the vector as a pointer
    /// @param requested_length New vector capacity
    /// @return                 @see ReceiveArea
    ReceiveArea prepare_receive_area(const std::size_t &requested_length)
    {
        if (receive_area_prepared_ || requested_length == 0U)
        {
            return {};
        }
        receive_capacity_ = std::min(requested_length, frame_buffer_.available());
        old_size_ = frame_buffer_.size();

        if (receive_capacity_ == 0U)
        {
            clear_receive_area_state();
            return {};
        }

        receive_area_prepared_ = true;
        frame_buffer_.uninitialized_resize(old_size_ + receive_capacity_);

        return {frame_buffer_.data() + old_size_, receive_capacity_};
    }

    /// @brief Used to add the new received data
    /// @param received_length
    /// @return                 true = succes,
    ///                         false = error
    bool commit_receive(const std::size_t &received_length)
    {
        if (!receive_area_prepared_)
        {
            return false;
        }

        if (received_length > receive_capacity_)
        {
            rollback_receive();
            return false;
        }

        frame_buffer_.uninitialized_resize(old_size_ + received_length);
        clear_receive_area_state();
        return true;
    }

    /// @brief Restores the last vector size
    /// @note               Used when the vector was increased,
    ///                     but the receive failed.
    void rollback_receive()
    {
        if (receive_area_prepared_)
        {
            frame_buffer_.uninitialized_resize(old_size_);
        }

        clear_receive_area_state();
    }

    // Copy-based input path for transports whose buffers are owned elsewhere,
    // e.g. TCP/UDP stacks. It is intentionally unavailable while a direct
    // receive area is owned by an asynchronous transfer.
    bool append_received_data(
        const std::uint8_t *data,
        std::size_t length)
    {
        if (receive_area_prepared_ ||
            (data == nullptr && length > 0U) ||
            length > frame_buffer_.available())
        {
            return false;
        }

        if (length > 0U)
        {
            frame_buffer_.insert(
                frame_buffer_.end(),
                data,
                data + length);
        }

        return true;
    }

    /// @brief Method to check the remaining bytes from a message
    /// @note                       Internally used to check the new message
    ///                             (and the remaining bytes inside the buffer)
    /// @param [out] id             Messageid
    /// @param [out] output         Output destination array
    /// @param [in] outputCapacity  Size of the array
    /// @param [out] outputSize     Received size
    /// @param [inout] frame        Frame Version format
    /// @return                     @see Frame_NS::CommError
    Frame_NS::CommError check(MessageId &id,
                              std::uint8_t *output,
                              std::size_t output_capacity,
                              std::size_t &output_size,
                              Frame_NS::frame_unpack_if &frame)
    {
        using namespace Frame_NS;

        output_size = 0U;

        if (receive_area_prepared_)
        {
            return CommError::InvalidState;
        }

        if (!output || output_capacity <= 0)
        {
            return (!output) ? CommError::InvalidArgument : CommError::InvalidLength;
        }

        if (frame_buffer_.empty())
        {
            return CommError::InvalidLength;
        }

        const CommError decodeResult = frame.decode(
            id,
            frame_buffer_.data(),
            frame_buffer_.size(),
            output,
            output_capacity,
            output_size);

        if (output_size > output_capacity)
        {
            frame_buffer_.clear();
            output_size = 0U;
            return CommError::ClassInternalBufferTooSmall;
        }
        switch (decodeResult)
        {
        case CommError::message_finished_buffer_not_empty:
        {
            const std::size_t consumed = frame.get_index();

            if ((consumed == 0U) || (consumed > frame_buffer_.size()))
            {
                frame_buffer_.clear();
                return CommError::InvalidLength;
            }

            frame_buffer_.erase(
                frame_buffer_.begin(),
                frame_buffer_.begin() + consumed);

            return decodeResult;
        }

        case CommError::message_unfinished:
            // The unpacker stores the intermediate frame state internally.
            // Therefore all bytes passed to decode() have been consumed and
            // must not be submitted again on the next call.
            frame_buffer_.clear();
            return decodeResult;

        case CommError::None:
            frame_buffer_.clear();
            return CommError::None;

        default:
            frame_buffer_.clear();
            return decodeResult;
        }
    }

    /// @brief Getter receive_area_prepared_
    /// @return size of the frame_buffer_
    [[nodiscard]] std::size_t buffered_size() const noexcept
    {
        return frame_buffer_.size();
    }

    /// @brief Getter frame_buffer_
    /// @return size of the frame_buffer_
    [[nodiscard]] bool receive_area_prepared() const noexcept
    {
        return receive_area_prepared_;
    }

private:
    void clear_receive_area_state() noexcept
    {
        old_size_ = 0U;
        receive_capacity_ = 0U;
        receive_area_prepared_ = false;
    }

    std::size_t old_size_{0U};         // Used to increase the vector size or restore the old size
    std::size_t receive_capacity_{0U}; //
    bool receive_area_prepared_{false};
};
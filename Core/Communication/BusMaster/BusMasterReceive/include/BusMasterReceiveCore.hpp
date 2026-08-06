#pragma once
#include <cstddef>

#include "frame.hpp"
#include "binary_container.hpp"
#include "etl/vector.h"

template <std::size_t MessageSize>
class BusMasterReceiveCore
{
    static_assert(MessageSize > 0U, "BusMasterReceiveCore requires a non-zero message buffer");

protected:
    std::array<std::uint8_t, MessageSize> message_buffer_{};

    // Die feste 100 bleibt bei dir bestehen.
    etl::vector<std::uint8_t, MessageSize> frame_buffer_{};

    struct ReceiveArea
    {
        std::uint8_t *data{nullptr};
        std::size_t capacity{0U};
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return data != nullptr && capacity > 0U;
        }
    };

    ReceiveArea prepare_receive_area(std::size_t requested_length)
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

    bool commit_receive(std::size_t received_length)
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

    Frame_NS::CommError check(
        MessageId &id,
        std::uint8_t *output,
        std::size_t output_capacity,
        std::size_t &output_size,
        Frame_NS::frame_unpack_if &frame)
    {
        using namespace Frame_NS;

        output_size = 0U;

        if (receive_area_prepared_)
        {
            return CommError::Busy;
        }

        if (output == nullptr || output_capacity <= 0)
        {
            return CommError::InvalidLength;
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

    [[nodiscard]] std::size_t buffered_size() const noexcept
    {
        return frame_buffer_.size();
    }

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

    std::size_t old_size_{0U};
    std::size_t receive_capacity_{0U};
    bool receive_area_prepared_{false};
};
#ifndef FRAME_V1_HPP
#define FRAME_V1_HPP

#include <cstdint>
#include <array>

#include "frame.hpp"
#include "binary_container.hpp"

/*
                Format
--------------------------------------------------------
| Start | Version | Message-ID | Länge | Payload | CRC |
--------------------------------------------------------
*/

struct frameV1_header
{
    using startbyte_size_t = std::uint8_t;
    static constexpr startbyte_size_t startbyte = 0xf7;

    using version_size_t = std::uint8_t;
    static constexpr version_size_t version_ = 1;

    using len_size_t = std::uint8_t;
    using messag_id_t = std::uint16_t;

    static constexpr std::size_t FrameHeadSize_ = sizeof(startbyte_size_t) +
                                                  sizeof(version_size_t) +
                                                  sizeof(messag_id_t) +
                                                  sizeof(len_size_t);
};

struct FrameHead
{
    frameV1_header::startbyte_size_t startbyte;
    frameV1_header::version_size_t version;
    frameV1_header::messag_id_t messageid;
    frameV1_header::len_size_t len;
};

template <typename CrcT>
class frameV1_pack : public Frame_NS::frame_pack_if
{
public:
    frameV1_header header_;

    static constexpr std::size_t crc_size = sizeof(CrcT);

private:
    crc_wrapper<CrcT> crc_;

public:
    explicit frameV1_pack(crc_wrapper<CrcT> crc)
        : crc_{crc}
    {
    }
    virtual Frame_NS::CommError encode(const MessageId &messageid,
                                       const std::uint8_t *payload,
                                       const std::size_t &payloadSize,
                                       std::uint8_t *output,
                                       const std::int16_t &outputCapacity,
                                       std::size_t &outputSize) const override
    {
        BinaryWriter writer(output, outputCapacity);
        frameV1_header::len_size_t len = payloadSize + header_.FrameHeadSize_ + crc_size;
        if (!writer.write(header_.startbyte) || !writer.write(header_.version_) || !writer.write(static_cast<uint16_t>(messageid)) || !writer.write(len))
        {
            return Frame_NS::CommError::BufferTooSmall;
        }
        for (size_t i = 0; i < payloadSize; i++)
        {
            if (!writer.write(payload[i]))
            {
                return Frame_NS::CommError::BufferTooSmall;
            }
        }
        CrcT checksum = crc_.code(output + 1, writer.size() - 1);
        if (!writer.write(checksum))
        {
            return Frame_NS::CommError::BufferTooSmall;
        }
        outputSize = writer.size();
        return Frame_NS::CommError::None;
    }
    virtual ~frameV1_pack() = default;
};

template <typename CrcT, std::size_t buffer_size = frameV1_header::FrameHeadSize_>
class frameV1_unpack : public Frame_NS::frame_unpack_if
{
private:
    static constexpr std::size_t crc_size = sizeof(CrcT);

    crc_wrapper<CrcT> crc_;
    FrameHead framehead_;
    std::size_t received_;
    bool head_received_;
    std::array<std::uint8_t, buffer_size> buffer_;
    std::size_t index_;

public:
    explicit frameV1_unpack(crc_wrapper<CrcT> crc)
        : crc_{crc},
          framehead_{},
          received_{},
          head_received_{false},
          buffer_{},
          index_{}
    {
    }
    virtual ~frameV1_unpack() = default;

    virtual Frame_NS::CommError decode(MessageId &messageid,
                                       const std::uint8_t *payload,
                                       const std::size_t &payloadSize,
                                       std::uint8_t *output,
                                       const std::int16_t &outputCapacity,
                                       std::size_t &outputSize) override
    {

        std::size_t input_received = 0;
        while (!head_received_ && received_ < frameV1_header::FrameHeadSize_ && input_received < payloadSize)
        {
            buffer_[received_] = payload[input_received++];
            if (buffer_[0] == frameV1_header::startbyte)
            {
                received_++;
            }
            if (received_ > buffer_size)
            {
                reset();
                return Frame_NS::CommError::ClassInternalBufferTooSmall;
            }
        }

        if (received_ >= frameV1_header::FrameHeadSize_ && !head_received_)
        {
            BinaryReader reader(buffer_.data(), buffer_.size());
            if (!reader.read(framehead_.startbyte) || !reader.read(framehead_.version) || !reader.read(framehead_.messageid) || !reader.read(framehead_.len))
            {
                reset();
                return Frame_NS::CommError::BufferTooSmall;
            }
            if (framehead_.startbyte != frameV1_header::startbyte || framehead_.version != frameV1_header::version_)
            {
                reset();
                return Frame_NS::CommError::InvalidFrame;
            }
            if (framehead_.len > outputCapacity)
            {
                reset();
                return Frame_NS::CommError::BufferTooSmall;
            }
            head_received_ = true;
        }

        if (!head_received_)
        {
            return (received_ > 0) ? Frame_NS::CommError::message_unfinished : Frame_NS::CommError::InvalidFrame;
        }

        while (input_received < payloadSize && received_ < framehead_.len)
        {
            output[received_++ - frameV1_header::FrameHeadSize_] = payload[input_received++];
        }

        if (received_ < framehead_.len)
        {
            return Frame_NS::CommError::message_unfinished;
        }

        // CRC
        CrcT crc = crc_.code(buffer_.data() + 1, buffer_.size() - 1);
        crc = crc_.update(output, crc, received_ - frameV1_header::FrameHeadSize_ - crc_size);
        CrcT crcReceived = 0;

        for (size_t i = 0; i < crc_size; i++)
        {
            crcReceived |=
                static_cast<CrcT>(output[received_ - frameV1_header::FrameHeadSize_ - crc_size + i])
                << (8U * i);
        }

        if (crc != crcReceived)
        {
            reset();
            return Frame_NS::CommError::CrcMismatch;
        }
        messageid = static_cast<MessageId>(framehead_.messageid);
        auto ret = (received_ >= framehead_.len && input_received < payloadSize) ? Frame_NS::CommError::message_finished_buffer_not_empty
                                                                                 : Frame_NS::CommError::None;
        index_ = (Frame_NS::CommError::message_finished_buffer_not_empty == ret) ? input_received : 0;
        outputSize = received_ - frameV1_header::FrameHeadSize_ - crc_size;
        reset();
        return ret;
    }

    virtual bool verify() const override { return true; };

    virtual std::size_t get_index() const { return index_; }

    virtual std::size_t get_MaxSize() const { return frameV1_header::FrameHeadSize_ + crc_size; }

    void reset()
    {
        framehead_.len = 0;
        framehead_.messageid = 0;
        framehead_.startbyte = 0;
        framehead_.version = 0;
        received_ = 0;
        head_received_ = false;
    }
};

#endif

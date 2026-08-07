#include <gtest/gtest.h>
#include <iostream>
#include <array>
#include <type_traits>

#include "crc.hpp"
#include "coder.hpp"
#include "mock_spi.hpp"
#include "archive_generic.hpp"
#include "archive_example.hpp"
#include "frameV1.hpp"

TEST(FrameV1, FramePack)
{
    std::array<uint8_t, 100> buffer{};
    std::array<uint8_t, 100> buffer_message{};
    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);

    frameV1_pack frame(coder);

    BinaryWriter write(buffer_message.data(), buffer_message.size());

    std::size_t len = 0;
    command.serialize(write);
    auto result = frame.encode(MessageTraits<ConfigureGeneric<12>>::id, buffer_message.data(), write.size(), buffer.data(), buffer.size(), len);
    int x = 0;

    // Test
    ASSERT_EQ(frameV1_header::startbyte, buffer[0]);
    ASSERT_EQ(frameV1_header::version_, buffer[1]);
    uint16_t id = (buffer[2] | buffer[3] << 8);
    ASSERT_EQ(static_cast<uint16_t>(MessageTraits<ConfigureGeneric<12>>::id), id);
    ASSERT_EQ(frameV1_header::FrameHeadSize_ + frame.crc_size + sizeof(command), buffer[4]);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    for (size_t i = 0; i < write.size(); i++)
    {
        ASSERT_EQ(buffer_message[i], buffer[i + frameV1_header::FrameHeadSize_]);
    }
    auto crc = coder.code(buffer.data() + 1, len - 1 - frame.crc_size);
    uint16_t crc_msg = (buffer[20] << 8) | buffer[19];
    ASSERT_EQ(crc_msg, crc);
}

TEST(FrameV1, PackUnpack)
{
    // Create buffers and messages
    std::array<uint8_t, 100> buffer{};
    std::array<uint8_t, 100> buffer_message{};
    ConfigureGeneric<12> message_decoded{};
    ConfigureGeneric<12> command{
        0xfA,
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}};

    // Set coder variables
    crc_wrapper<std::uint16_t> coder(0xFFFFu, 0x1021u);

    frameV1_pack frame(coder);

    BinaryWriter write(buffer_message.data(), buffer_message.size());

    std::size_t len = 0;
    command.serialize(write);
    auto result = frame.encode(MessageTraits<ConfigureGeneric<12>>::id, buffer_message.data(), write.size(), buffer.data(), buffer.size(), len);

    ASSERT_EQ(result, Frame_NS::CommError::None);
    int x = 0;

    frameV1_unpack DUT(coder);

    MessageId id;

    std::array<std::uint8_t, 100> decoded_message{};
    result = DUT.decode(id, buffer.data(), len, decoded_message.data(), decoded_message.size(), len);
    ASSERT_EQ(result, Frame_NS::CommError::None);

    BinaryReader reader(decoded_message.data(), len);

    message_decoded.serialize(reader);

    ASSERT_EQ(command.command, message_decoded.command);
    for (size_t i = 0; i < 12; i++)
    {
        ASSERT_EQ(command.Payload[i], message_decoded.Payload[i]);
    }
}

TEST(FrameV1, FrameUnpackPartialReceive)
{
    // --------------------------------------------------------
    // Nachricht erzeugen
    // --------------------------------------------------------

    ConfigureGeneric<12> command{
        0x00FA,
        {0x01, 0x02, 0x03, 0x04,
         0x05, 0x06, 0x07, 0x08,
         0x09, 0x0A, 0x0B, 0x0C}};

    ConfigureGeneric<12> messageDecoded{};

    std::array<std::uint8_t, 100> payloadBuffer{};
    std::array<std::uint8_t, 100> frameBuffer{};
    std::array<std::uint8_t, 100> decodedPayload{};

    // --------------------------------------------------------
    // Nachricht serialisieren
    // --------------------------------------------------------

    BinaryWriter writer(
        payloadBuffer.data(),
        payloadBuffer.size());

    ASSERT_TRUE(command.serialize(writer));

    // Bei uint32_t solltest du auch ein passendes
    // 32-Bit-Polynom verwenden.
    crc_wrapper<std::uint32_t> coder(
        0xFFFFFFFFu,
        0x04C11DB7u);

    frameV1_pack framePacker(coder);

    std::size_t frameLength = 0;

    const auto encodeResult = framePacker.encode(
        MessageTraits<ConfigureGeneric<12>>::id,
        payloadBuffer.data(),
        writer.size(),
        frameBuffer.data(),
        frameBuffer.size(),
        frameLength);

    ASSERT_EQ(
        encodeResult,
        Frame_NS::CommError::None);

    // --------------------------------------------------------
    // Frame stückweise empfangen
    // --------------------------------------------------------

    frameV1_unpack frameUnpacker(coder);

    MessageId receivedId{};
    std::size_t decodedPayloadLength = 0;
    std::size_t inputOffset = 0;

    // Absichtlich unterschiedliche Empfangsgrößen.
    constexpr std::array<std::size_t, 6> chunkSizes{
        2, 1, 4, 3, 5, 100};

    Frame_NS::CommError decodeResult =
        Frame_NS::CommError::message_unfinished;

    for (const std::size_t requestedChunkSize : chunkSizes)
    {
        if (inputOffset >= frameLength)
        {
            break;
        }

        const std::size_t remaining =
            frameLength - inputOffset;

        const std::size_t currentChunkSize =
            requestedChunkSize < remaining
                ? requestedChunkSize
                : remaining;

        decodeResult = frameUnpacker.decode(
            receivedId,
            frameBuffer.data() + inputOffset,
            currentChunkSize,
            decodedPayload.data(),
            decodedPayload.size(),
            decodedPayloadLength);

        inputOffset += currentChunkSize;

        if (inputOffset < frameLength)
        {
            ASSERT_EQ(
                decodeResult,
                Frame_NS::CommError::message_unfinished);
        }
    }

    ASSERT_EQ(inputOffset, frameLength);

    ASSERT_EQ(
        decodeResult,
        Frame_NS::CommError::None);

    ASSERT_EQ(
        receivedId,
        MessageTraits<ConfigureGeneric<12>>::id);

    ASSERT_EQ(
        decodedPayloadLength,
        writer.size());

    // --------------------------------------------------------
    // Payload wieder in das Struct deserialisieren
    // --------------------------------------------------------

    BinaryReader reader(
        decodedPayload.data(),
        decodedPayloadLength);

    ASSERT_TRUE(messageDecoded.serialize(reader));
    ASSERT_TRUE(reader.finished());

    ASSERT_EQ(command.command, messageDecoded.command);
    ASSERT_EQ(command.Payload, messageDecoded.Payload);
}

class FrameV1DecodeTest : public ::testing::Test
{
protected:
    using CrcT = std::uint16_t;

    crc_wrapper<CrcT> coder_{
        0xFFFFu,
        0x1021u};

    ConfigureGeneric<12> command_{
        0x00FA,
        {0x01, 0x02, 0x03, 0x04,
         0x05, 0x06, 0x07, 0x08,
         0x09, 0x0A, 0x0B, 0x0C}};

    std::array<std::uint8_t, 100> payload_{};
    std::array<std::uint8_t, 100> frame_{};

    std::size_t payloadSize_{};
    std::size_t frameSize_{};

    void SetUp() override
    {
        BinaryWriter writer(
            payload_.data(),
            payload_.size());

        ASSERT_TRUE(command_.serialize(writer));
        payloadSize_ = writer.size();

        frameV1_pack packer(coder_);

        ASSERT_EQ(
            packer.encode(
                MessageTraits<ConfigureGeneric<12>>::id,
                payload_.data(),
                payloadSize_,
                frame_.data(),
                frame_.size(),
                frameSize_),
            Frame_NS::CommError::None);
    }
};

TEST_F(FrameV1DecodeTest, PartialHeader)
{
    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::size_t outputSize{};

    constexpr std::size_t firstPartSize = 2;

    auto result = decoder.decode(
        id,
        frame_.data(),
        firstPartSize,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::message_unfinished);

    result = decoder.decode(
        id,
        frame_.data() + firstPartSize,
        frameSize_ - firstPartSize,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(result, Frame_NS::CommError::None);
    EXPECT_EQ(
        id,
        MessageTraits<ConfigureGeneric<12>>::id);
    EXPECT_EQ(outputSize, payloadSize_);
}

TEST_F(FrameV1DecodeTest, PartialPayload)
{
    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::size_t outputSize{};

    const std::size_t firstPartSize =
        frameV1_header::FrameHeadSize_ + 3;

    auto result = decoder.decode(
        id,
        frame_.data(),
        firstPartSize,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::message_unfinished);

    result = decoder.decode(
        id,
        frame_.data() + firstPartSize,
        frameSize_ - firstPartSize,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(result, Frame_NS::CommError::None);
    EXPECT_EQ(outputSize, payloadSize_);
}

TEST_F(FrameV1DecodeTest, InvalidStartByte)
{
    frame_[0] ^= 0xFFU;

    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::size_t outputSize{};

    const auto result = decoder.decode(
        id,
        frame_.data(),
        frameSize_,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::InvalidFrame);
}

TEST_F(FrameV1DecodeTest, InvalidVersion)
{
    frame_[1]++;

    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::size_t outputSize{};

    const auto result = decoder.decode(
        id,
        frame_.data(),
        frameSize_,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::InvalidFrame);
}

TEST_F(FrameV1DecodeTest, DetectsCrcError)
{
    // Erstes Payloadbyte verändern
    frame_[frameV1_header::FrameHeadSize_] ^= 0x01U;

    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::size_t outputSize{};

    const auto result = decoder.decode(
        id,
        frame_.data(),
        frameSize_,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::CrcMismatch);
}

TEST_F(FrameV1DecodeTest, DetectsSmallOutputBuffer)
{
    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 2> output{};
    std::size_t outputSize{};

    const auto result = decoder.decode(
        id,
        frame_.data(),
        frameSize_,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::BufferTooSmall);
}

TEST_F(FrameV1DecodeTest, RecoversAfterCrcError)
{
    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::size_t outputSize{};

    auto damagedFrame = frame_;
    damagedFrame[frameV1_header::FrameHeadSize_] ^= 0x01U;

    EXPECT_EQ(
        decoder.decode(
            id,
            damagedFrame.data(),
            frameSize_,
            output.data(),
            output.size(),
            outputSize),
        Frame_NS::CommError::CrcMismatch);

    outputSize = 0;

    EXPECT_EQ(
        decoder.decode(
            id,
            frame_.data(),
            frameSize_,
            output.data(),
            output.size(),
            outputSize),
        Frame_NS::CommError::None);
}

TEST_F(FrameV1DecodeTest, DetectsRemainingInputData)
{
    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::size_t outputSize{};

    auto frameWithExtraByte = frame_;

    ASSERT_LT(frameSize_, frameWithExtraByte.size());

    // Zusätzliches Byte hinter dem vollständigen Frame
    frameWithExtraByte[frameSize_] = 0xAA;

    const std::size_t receivedSize = frameSize_ + 1;

    const auto result = decoder.decode(
        id,
        frameWithExtraByte.data(),
        receivedSize,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::message_finished_buffer_not_empty);
}

TEST_F(FrameV1DecodeTest, DetectsStart)
{
    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::array<std::uint8_t, 100> frameWithExtraByte{};
    std::size_t outputSize{};

    constexpr std::size_t offset = 6;
    frameSize_ += offset;
    for (size_t i = offset; i < frameSize_; i++)
    {
        frameWithExtraByte[i] = frame_[i - offset];
    }

    ASSERT_LT(frameSize_, frameWithExtraByte.size());

    const std::size_t receivedSize = frameSize_;

    const auto result = decoder.decode(
        id,
        frameWithExtraByte.data(),
        receivedSize,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::None);
}

TEST_F(FrameV1DecodeTest, DetectsNoStartByte)
{
    frameV1_unpack decoder(coder_);

    MessageId id{};
    std::array<std::uint8_t, 100> output{};
    std::array<std::uint8_t, 100> frameNoSTartByte{};
    std::size_t outputSize{};

    const std::size_t receivedSize = frameSize_;

    const auto result = decoder.decode(
        id,
        frameNoSTartByte.data(),
        receivedSize,
        output.data(),
        output.size(),
        outputSize);

    EXPECT_EQ(
        result,
        Frame_NS::CommError::InvalidFrame);
}
#include <gtest/gtest.h>

#include <rescuevision/video/frame_source.hpp>

#include <stdexcept>
#include <string>
#include <cstdint>
#include <chrono>

const std::string fixture_path =
    std::string(RESCUEVISION_TEST_FIXTURE_DIR)
    + "/five_frames.avi";

using rescuevision::video::FrameSource;

TEST(FrameSourceTest, RejectsInvalidVideoPath)
{
    const std::string invalid_path =
        "/nonexistent/rescuevision_video.mp4";

    EXPECT_THROW(
        FrameSource source(invalid_path),
        std::runtime_error
    );
}

TEST(FrameSourceTest, ReadsSequentialFrames)
{
    const std::string video_path = fixture_path;

    FrameSource source(video_path);

    rescuevision::video::VideoFrame frame;

    for (std::uint64_t expected_index = 0;
         expected_index < 5;
         ++expected_index)
    {
        const auto status = source.read(frame);

        ASSERT_EQ(
            status,
            rescuevision::video::ReadStatus::FrameReady
        );

        ASSERT_FALSE(frame.image.empty());

        EXPECT_EQ(frame.metadata.frame_index, expected_index);

        EXPECT_EQ(frame.image.cols, 320);
        EXPECT_EQ(frame.image.rows, 240);
    }
}

TEST(FrameSourceTest, IdentifiesUnreliableTimestamps)
{
    FrameSource source(fixture_path);
    rescuevision::video::VideoFrame frame;

    using rescuevision::core::TimestampStatus;
    using rescuevision::video::ReadStatus;

    const TimestampStatus expected_statuses[] = {
        TimestampStatus::DecoderProvided,
        TimestampStatus::DecoderProvided,
        TimestampStatus::DecoderProvided,
        TimestampStatus::Unavailable,
        TimestampStatus::Unavailable
    };

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_EQ(source.read(frame), ReadStatus::FrameReady);

        EXPECT_EQ(
            frame.metadata.timestamp_status,
            expected_statuses[i]
        );
    }
}

TEST(FrameSourceTest, ClearsFrameAtEndOfStream)
{
    const std::string video_path = fixture_path;

    FrameSource source(video_path);
    rescuevision::video::VideoFrame frame;

    std::uint64_t decoded_count = 0;

    while (true)
    {
        const auto status = source.read(frame);

        if (status == rescuevision::video::ReadStatus::FrameReady)
        {
            ++decoded_count;
            continue;
        }

        ASSERT_EQ(
            status,
            rescuevision::video::ReadStatus::EndOfStream
        );

        break;
    }

    //EXPECT_EQ(decoded_count, 5u);
    EXPECT_TRUE(frame.image.empty());
    EXPECT_TRUE(frame.metadata.source_id.empty());
    EXPECT_EQ(frame.metadata.frame_index, 0u);
    EXPECT_EQ(
        frame.metadata.timestamp_status,
        rescuevision::core::TimestampStatus::Unavailable
    );
}
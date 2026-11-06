#include <gtest/gtest.h>

#include <rescuevision/core/frame_metadata.hpp>

#include <chrono>
#include <string>

using rescuevision::core::FrameMetadata;
using rescuevision::core::TimestampStatus;

TEST(FrameMetadataTest, TimestampUnavailableByDefault)
{
    FrameMetadata frame{};

    EXPECT_EQ(
        frame.timestamp_status,
        TimestampStatus::Unavailable
    );

    EXPECT_EQ(
        frame.video_timestamp,
        std::chrono::microseconds{0}
    );
}

TEST(FrameMetadataTest, StoresEstimatedTimestamp)
{
    FrameMetadata frame{};

    frame.video_timestamp = std::chrono::seconds{4};
    frame.timestamp_status = TimestampStatus::Estimated;

    EXPECT_EQ(
        frame.video_timestamp,
        std::chrono::microseconds{4000000}
    );

    EXPECT_EQ(
        frame.timestamp_status,
        TimestampStatus::Estimated
    );
}

TEST(FrameMetadataTest, HandlesBoundaryValues)
{
    // Test the first frame of a recording
    FrameMetadata first_frame{
        "mission_video_001",
        0,
        std::chrono::microseconds{0}
    };

    EXPECT_EQ(first_frame.frame_index, 0u);
    EXPECT_EQ(
        first_frame.video_timestamp,
        std::chrono::microseconds{0}
    );

    // Test a large frame index
    FrameMetadata large_frame{
        "mission_video_001",
        1000000,
        std::chrono::seconds{3600}
    };

    EXPECT_EQ(large_frame.frame_index, 1000000u);
    EXPECT_EQ(
        large_frame.video_timestamp,
        std::chrono::microseconds{3600000000LL}
    );
}

TEST(FrameMetadataTest, StoresExpectedValues)
{
    FrameMetadata frame{
        "mission_video_001",
        300,
        std::chrono::seconds{10}
    };

    EXPECT_EQ(frame.source_id, "mission_video_001");
    EXPECT_EQ(frame.frame_index, 300u);

    EXPECT_EQ(
        frame.video_timestamp,
        std::chrono::microseconds{10000000}
    );
}


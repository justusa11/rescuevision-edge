#include <rescuevision/video/frame_source.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace rescuevision::video
{

FrameSource::FrameSource(const std::string& video_path)
      // Attempts to open recordign  
    : capture_(video_path),
      source_id_(video_path)
{
    if (!capture_.isOpened())
    {
        throw std::runtime_error(
            "Failed to open video: " + video_path
        );
    }

    std::cout
        << "Video backend: "
        << capture_.getBackendName()
        << '\n';
}

bool FrameSource::is_open() const
{
    return capture_.isOpened();
}

ReadStatus FrameSource::read(VideoFrame& frame)
{
    // Reset output before attempting to decode.
    frame = VideoFrame{};

    // Verify that the capture is still open.
    if (!capture_.isOpened())
    {
        return ReadStatus::Error;
    }

    // Temporary decoded image.
    cv::Mat image;

    // Attempt to decode one frame.
    const bool success = capture_.read(image);

    if (!success || image.empty())
    {
        const double current_position =
            capture_.get(cv::CAP_PROP_POS_FRAMES);
        
        const double reported_total =
            capture_.get(cv::CAP_PROP_FRAME_COUNT);

        if (
            std::isfinite(current_position) &&
            std::isfinite(reported_total) &&
            reported_total > 0.0 &&
            current_position >= reported_total
        )
        {
            return ReadStatus::EndOfStream;
        }

        return ReadStatus::UnknownFailure;
    }

    // Frame metadata and successful return
    // will be implemented in the next step.
    frame.image = std::move(image);

    // Associate metadata with the decoded frame.
    frame.metadata.source_id = source_id_;
    frame.metadata.frame_index = frame_index_;

    using rescuevision::core::TimestampStatus;

    // Default status remains unavailable.
    frame.metadata.timestamp_status =
        TimestampStatus::Unavailable;

    const double timestamp_ms =
        capture_.get(cv::CAP_PROP_POS_MSEC);

    // Validate the decoder-provided timestamp.
    if (std::isfinite(timestamp_ms) && timestamp_ms >= 0.0)
    {
        const auto candidate_timestamp =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::duration<double, std::milli>{timestamp_ms}
            );

        const bool is_first_frame = frame_index_ == 0;

        const bool is_increasing =
            previous_timestamp_.has_value() &&
            candidate_timestamp > previous_timestamp_.value();

        if (is_first_frame || is_increasing)
        {
            frame.metadata.video_timestamp = candidate_timestamp;
            frame.metadata.timestamp_status =
                TimestampStatus::DecoderProvided;

            previous_timestamp_ = candidate_timestamp;
        }
    }

    // Temporary diagnostics for the first ten frames.
    if (frame_index_ < 10)
    {
        const double position = 
            capture_.get(cv::CAP_PROP_POS_FRAMES);

        const double fps =
            capture_.get(cv::CAP_PROP_FPS);

        std::cout
            << "Frame: " << frame_index_
            << " | Position: " << position
            << " | FPS: " << fps
            << " | Timestamp: " << timestamp_ms
            << " ms\n";
    }
    
    ++frame_index_;

    return ReadStatus::FrameReady;
}

}
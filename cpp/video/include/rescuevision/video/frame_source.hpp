#pragma once

#include <chrono>
#include <optional>

#include <rescuevision/video/video_frame.hpp>
#include <opencv2/videoio.hpp>

#include <cstdint>
#include <string>

namespace rescuevision::video
{

enum class ReadStatus
{
    FrameReady,
    EndOfStream,
    Error,
    UnknownFailure
};

class FrameSource
{
public:
    explicit FrameSource(const std::string& video_path);

    // Decode the next frame and populate metadata.
    ReadStatus read(VideoFrame& frame);

    [[nodiscard]] bool is_open() const;

private:
    cv::VideoCapture capture_;
    std::string source_id_;
    std::uint64_t frame_index_ = 0;
    std::optional<std::chrono::microseconds> previous_timestamp_;
};

}
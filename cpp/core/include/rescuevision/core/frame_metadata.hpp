// Prevents repeated inclusion within one translation unit
#pragma once

#include <chrono>
#include <cstdint>
#include <string>

// Separates definitions from other libraries
namespace rescuevision::core
{

enum class TimestampStatus
{
    Unavailable,
    DecoderProvided,
    Estimated
};

struct FrameMetadata
{
    std::string source_id;
    std::uint64_t frame_index{};    // Zero-based decoded frame index

    std::chrono::microseconds video_timestamp{};    //Typed duration

    TimestampStatus timestamp_status =
        TimestampStatus::Unavailable;
};


}
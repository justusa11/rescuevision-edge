#pragma once

#include <opencv2/core/mat.hpp>

#include <rescuevision/core/frame_metadata.hpp>

// Ownership of Data Type clear
namespace rescuevision::video 
{

struct VideoFrame
{
    cv::Mat image;
    rescuevision::core::FrameMetadata metadata;
};

}
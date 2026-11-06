
#include <iostream>
#include <exception>
#include <cstdint>
#include <string>

#include <rescuevision/video/frame_source.hpp>

int main()
{
    std::cout << "RescueVision Edge\n";
    std::cout << "RV-003D: Video Ingestion\n";

    const std::string video_path =
        "/home/anoru/Visdrone/videos/drone_traffic.mp4";

    try
    {
        rescuevision::video::FrameSource source(video_path);
        rescuevision::video::VideoFrame frame;

        std::uint64_t frames_decoded = 0;

        while (true)
        {
            const auto status = source.read(frame);

            if (status == rescuevision::video::ReadStatus::FrameReady)
            {
                ++frames_decoded;
                continue;
            }

            if (status == rescuevision::video::ReadStatus::EndOfStream)
            {
                std::cout << "End of stream reached\n";
                break;
            }

            std::cerr
                << "Read stopped: error or unverified EOF\n";

            std::cout << "Frames decoded before stopping: "
                      << frames_decoded << '\n';

            return 1;
        }

        std::cout << "Total decoded frames: "
                  << frames_decoded << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

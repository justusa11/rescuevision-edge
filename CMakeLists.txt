
cmake_minimum_required(VERSION 3.24)

project(
    RescueVisionEdge
    VERSION 0.1.0
    LANGUAGES CXX
)

# --------------------------------------------------
# C++ configuration
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# External dependencies
# --------------------------------------------------

find_package(OpenCV 4 REQUIRED COMPONENTS core videoio imgcodecs)

# --------------------------------------------------
# RescueVision libraries
# --------------------------------------------------

add_subdirectory(cpp/core)
add_subdirectory(cpp/video)

# --------------------------------------------------
# Replay CLI
# --------------------------------------------------

add_executable(
    rescuevision_replay
    cpp/apps/replay_cli/main.cpp
)

target_link_libraries(
    rescuevision_replay
    PRIVATE
        rescuevision_core
        rescuevision_video
)

target_compile_options(
    rescuevision_replay
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

# --------------------------------------------------
# Automated testing
# --------------------------------------------------

include(CTest)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)

    add_executable(
        core_tests
        tests/unit/frame_metadata_test.cpp
    )

    target_link_libraries(
        core_tests
        PRIVATE
            rescuevision_core
            GTest::gtest_main
    )

    add_test(
        NAME core_tests
        COMMAND core_tests
    )

    add_executable(
        video_tests
        tests/integration/frame_source_test.cpp
    )

    target_link_libraries(
        video_tests
        PRIVATE
            rescuevision_video
            GTest::gtest_main
    )

    target_compile_definitions(
        video_tests
        PRIVATE
            RESCUEVISION_TEST_FIXTURE_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures"
    )

    add_test(
        NAME video_tests
        COMMAND video_tests
    )
endif()

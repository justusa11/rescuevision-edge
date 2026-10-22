cmake_minimum_required(VERSION 3.24) # Specifies the minimum supported CMake version

project(                             # Project identity, version and language
    RescueVisionEdge
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)            # Language Standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # Prevent falling back to older standard
set(CMAKE_CXX_EXTENSIONS OFF)         # Requests standard C++ rather than compiler specific language extensions

add_executable(                       # Defines the program CMake must build
    rescuevision_replay
    cpp/apps/replay_cli/main.cpp
)

target_compile_options(               # Enables useful compiler warnings for that target
    rescuevision_replay
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_subdirectory(
    cpp/core
)

target_link_libraries(
    rescuevision_replay
    PRIVATE
        rescuevision_core
)

include(CTest)                      # Enables CTest support and the BUILD_TESTING option

if(BUILD_TESTING)
    find_package(GTest REQUIRED)    # Locates GoogleTest

    add_executable(                 # Compiles the test program
        core_tests
        tests/unit/frame_metadata_test.cpp
    )

    target_link_libraries(          # Connects the test to the core headers and GoogleTest
        core_tests
        PRIVATE
            rescuevision_core
            GTest::gtest_main
    )

    add_test(                       # Registers the executable with CTest
        NAME core_tests
        COMMAND core_tests
    )
endif()

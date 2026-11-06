add_library(
    rescuevision_video STATIC
    src/frame_source.cpp
)

target_include_directories(
    rescuevision_video
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    rescuevision_video
    PUBLIC
        rescuevision_core
        ${OpenCV_LIBS}
)

target_compile_features(
    rescuevision_video
    PUBLIC
        cxx_std_20
)
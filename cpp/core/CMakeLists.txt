add_library(rescuevision_core INTERFACE)

target_include_directories(
    rescuevision_core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
    rescuevision_core
    INTERFACE
        cxx_std_20
)

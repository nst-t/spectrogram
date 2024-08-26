cmake_minimum_required(VERSION 3.15)
project(spectrogram)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add global compile options to suppress -Wunused-result warning
add_compile_options(-Wno-unused-result)

find_package(lz4 REQUIRED)
find_package(mcap REQUIRED)
find_package(cargs REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PNG REQUIRED)

# Define sources and headers for the spectrogram executable
set(SPECTROGRAM_SOURCES
    src/nst_main.c
    src/math3d/math_3d.c
    src/quaternion/quaternion.c
    src/nelder_mead/nelder_mead.c
    src/spectrogram.cpp
    src/image_utils/image_utils.c
)

# Define the spectrogram executable target
add_executable(spectrogram ${SPECTROGRAM_SOURCES})
target_link_libraries(spectrogram lz4::lz4)
target_link_libraries(spectrogram ${CONAN_LIBS} m)
target_link_libraries(spectrogram mcap::mcap)
target_link_libraries(spectrogram cargs::cargs)
target_link_libraries(spectrogram nlohmann_json::nlohmann_json)
target_link_libraries(spectrogram PNG::PNG)

# Define sources for the synthetic executable
set(SYNTHETIC_SOURCES
    src/synthetic.cpp
    src/simple_fusion/simple_fusion.c
)

# Define the synthetic executable target
add_executable(synthetic ${SYNTHETIC_SOURCES})
target_link_libraries(synthetic m)
target_link_libraries(synthetic mcap::mcap)
target_link_libraries(synthetic cargs::cargs)
target_link_libraries(synthetic nlohmann_json::nlohmann_json)
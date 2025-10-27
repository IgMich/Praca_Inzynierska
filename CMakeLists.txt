cmake_minimum_required(VERSION 3.10.0)
project(PitchDetection VERSION 0.1.0 LANGUAGES C)

add_executable(PitchDetection
    main.c
    radix2_dit.c
    wavformat.c
    pitch_detection.c
    audio_spectrum.c
)

set(WAV_DIR "${CMAKE_SOURCE_DIR}/wav")
set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command(
    TARGET PitchDetection POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${WAV_DIR}"
            "${OUTPUT_DIR}/wav"
    COMMENT "Kopiowanie katalogu wav do katalogu binariów..."
)

include(CTest)
enable_testing()


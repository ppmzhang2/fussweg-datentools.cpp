cmake_minimum_required(VERSION 3.27)

project(FusswegDatentools VERSION 1.0.0)

find_package(exiv2 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

# # set linker flags to statically link libstdc++
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
# # set linker flags to statically link libc
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")

# define sources and headers
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/exif.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/exif_json.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/path_finder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/optical_flow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pov.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/annot.cpp"
)
set(HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/exif.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/exif_json.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/path_finder.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/optical_flow.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/pov.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/annot.hpp"
)

# Create the executable
add_executable(FusswegDatentools
    ${SOURCES}
    ${HEADERS}
)

# Set the runtime output directory to be inside the build directory
set_target_properties(FusswegDatentools PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Configure the file into the build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in"
    "${CMAKE_BINARY_DIR}/include/config.h"
)

target_link_libraries(FusswegDatentools PRIVATE
    Exiv2::exiv2lib
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

target_include_directories(FusswegDatentools PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/include" # Ensures config.h can be found
)

# Tests
enable_testing()
add_executable(FDTTest
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_exif.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/exif.cpp"
)
target_include_directories(FDTTest PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/include" # Ensures config.h can be found
)
target_link_libraries(FDTTest PRIVATE
    nlohmann_json::nlohmann_json
    Exiv2::exiv2lib
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME FDTTestSuite COMMAND FDTTest)

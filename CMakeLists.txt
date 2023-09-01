cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file"
) # the VCPKG is kept as a sibling project to this one

project(FusswegDatentools VERSION 1.0.0)

find_package(exiv2 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable(FusswegDatentools
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/exif.cpp"
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
    exiv2lib
)

target_include_directories(FusswegDatentools PUBLIC
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
    exiv2lib
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME FDTTestSuite COMMAND FDTTest)

# # TODO: Add compile options: libc++ and libc++abi. May need to add options
# #       for dependencies (e.g. exiv2) before adding these options.
# add_compile_options(-stdlib=libc++)
# # Add link options
# add_link_options(-stdlib=libc++)
# # Optionally add include directories for libc++ headers
# include_directories(/path/to/libc++/include)
# # Optionally add link directories for libc++ libraries
# link_directories(/path/to/libc++/lib)

# Link against libraries such as libdl, libc, etc.
# TODO: maybe libc++ and libc++abi?
link_directories(/usr/lib/x86_64-linux-gnu)

# -----------------------------------------------------------------------------
# OpenCV
# -----------------------------------------------------------------------------
set(OpenCV_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/opencv-install)
set(OpenCV_INC_DIR ${OpenCV_DIR}/include/opencv4)
set(OpenCV_LIB_DIR ${OpenCV_DIR}/lib)
set(OpenCV_LIB3RD_DIR ${OpenCV_DIR}/lib/opencv4/3rdparty)
set(ZLIBNG_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/zlib-ng-install)
set(ZLIBNG_INC_DIR ${ZLIBNG_DIR}/include)
set(ZLIBNG_LIB_DIR ${ZLIBNG_DIR}/lib)
set(PNG_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/libpng-install)
set(PNG_INC_DIR ${PNG_DIR}/include)
set(PNG_LIB_DIR ${PNG_DIR}/lib)

# Include directories
include_directories(${OpenCV_INC_DIR})
include_directories(${ZLIBNG_INC_DIR})
include_directories(${PNG_INC_DIR})

# Find the libraries
find_library(ZLIBNG NAMES z HINTS ${ZLIBNG_LIB_DIR})
find_library(PNG NAMES png HINTS ${PNG_LIB_DIR})

find_library(ILMIMF NAMES IlmImf HINTS ${OpenCV_LIB3RD_DIR})
find_library(ITTNOTIFY NAMES ittnotify HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBJPEGTURBO NAMES libjpeg-turbo HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBOPENJP2 NAMES libopenjp2 HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBTIFF NAMES libtiff HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBWEBP NAMES libwebp HINTS ${OpenCV_LIB3RD_DIR})

find_library(OPENCV_CORE NAMES opencv_core HINTS ${OpenCV_LIB_DIR})
find_library(OPENCV_IMGCODECS NAMES opencv_imgcodecs HINTS ${OpenCV_LIB_DIR})
find_library(OPENCV_IMGPROC NAMES opencv_imgproc HINTS ${OpenCV_LIB_DIR})
find_library(OPENCV_VIDEO NAMES opencv_video HINTS ${OpenCV_LIB_DIR})

# -----------------------------------------------------------------------------
# Exiv2
# -----------------------------------------------------------------------------
set(EXIV2_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/exiv2-install)
set(EXIV2_INC_DIR ${EXIV2_DIR}/include)
set(EXIV2_LIB_DIR ${EXIV2_DIR}/lib)

# Include directories
include_directories(${EXIV2_INC_DIR})

# Find the libraries
find_library(EXIV2 NAMES exiv2 HINTS ${EXIV2_LIB_DIR})

# -----------------------------------------------------------------------------
# nlohmann_json
# -----------------------------------------------------------------------------
set(JSON_SRC_DIR "${FusswegDatentools_SOURCE_DIR}/contrib/nlohmann_json")
set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(${JSON_SRC_DIR})

# -----------------------------------------------------------------------------
# GTest
# -----------------------------------------------------------------------------
add_subdirectory(${FusswegDatentools_SOURCE_DIR}/contrib/googletest-cmake)

# -----------------------------------------------------------------------------
# System and executable
# -----------------------------------------------------------------------------
find_package(Threads REQUIRED)

# define sources and headers
set(SOURCES
    "${FusswegDatentools_SOURCE_DIR}/src/main.cpp"
    "${FusswegDatentools_SOURCE_DIR}/src/exif.cpp"
    "${FusswegDatentools_SOURCE_DIR}/src/exif_json.cpp"
    "${FusswegDatentools_SOURCE_DIR}/src/path_finder.cpp"
    "${FusswegDatentools_SOURCE_DIR}/src/optical_flow.cpp"
    "${FusswegDatentools_SOURCE_DIR}/src/pov.cpp"
    "${FusswegDatentools_SOURCE_DIR}/src/annot.cpp"
)
set(HEADERS
    "${FusswegDatentools_SOURCE_DIR}/include/exif.hpp"
    "${FusswegDatentools_SOURCE_DIR}/include/exif_json.hpp"
    "${FusswegDatentools_SOURCE_DIR}/include/path_finder.hpp"
    "${FusswegDatentools_SOURCE_DIR}/include/optical_flow.hpp"
    "${FusswegDatentools_SOURCE_DIR}/include/pov.hpp"
    "${FusswegDatentools_SOURCE_DIR}/include/annot.hpp"
)

# Create the executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)
# Set the runtime output directory to be inside the build directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${FusswegDatentools_BINARY_DIR}/bin"
)

# Configure the file into the build directory
configure_file(
    "${FusswegDatentools_SOURCE_DIR}/include/config.h.in"
    "${FusswegDatentools_BINARY_DIR}/include/config.h"
)
# Include directories
target_include_directories(FusswegDatentools PRIVATE
    "${FusswegDatentools_SOURCE_DIR}/include"
    "${FusswegDatentools_BINARY_DIR}/include" # Ensures config.h can be found
)
# Link libraries
target_link_libraries(${PROJECT_NAME}
    c
    dl
    Threads::Threads
    ${ZLIBNG}
    ${PNG}
    ${ILMIMF}
    ${ITTNOTIFY}
    ${LIBJPEGTURBO}
    ${LIBOPENJP2}
    ${LIBTIFF}
    ${LIBWEBP}
    ${OPENCV_CORE}
    ${OPENCV_IMGCODECS}
    ${OPENCV_IMGPROC}
    ${OPENCV_VIDEO}
    ${EXIV2}
    nlohmann_json::nlohmann_json
)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
enable_testing()
add_executable(FDTTest
    "${FusswegDatentools_SOURCE_DIR}/tests/test_exif.cpp"
    "${FusswegDatentools_SOURCE_DIR}/src/exif.cpp"
)
target_include_directories(FDTTest PRIVATE
    "${FusswegDatentools_SOURCE_DIR}/include"
    "${FusswegDatentools_BINARY_DIR}/include" # Ensures config.h can be found
)
target_link_libraries(FDTTest PRIVATE
    c
    dl
    Threads::Threads
    ${ZLIBNG}
    ${PNG}
    ${ILMIMF}
    ${ITTNOTIFY}
    ${LIBJPEGTURBO}
    ${LIBOPENJP2}
    ${LIBTIFF}
    ${LIBWEBP}
    ${OPENCV_CORE}
    ${OPENCV_IMGCODECS}
    ${OPENCV_IMGPROC}
    ${OPENCV_VIDEO}
    ${EXIV2}
    nlohmann_json::nlohmann_json
    gtest
    gtest_main
)
add_test(NAME FDTTestSuite COMMAND FDTTest)

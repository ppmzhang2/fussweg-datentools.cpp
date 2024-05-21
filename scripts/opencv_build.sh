#!/bin/bash

# Get the directory where the script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source common functions
. "${SCRIPT_DIR}/common_func.sh"

# Define the variables
OCV_BUILD_DIR="${SCRIPT_DIR}/../build_contrib/opencv-build"
OCV_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/opencv-install"
OCV_SRC_DIR="${SCRIPT_DIR}/../contrib/opencv"
OCV_CONTRIB_SRC_DIR="${SCRIPT_DIR}/../contrib/opencv_contrib"
ZLIB_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/zlib-ng-install"
ZLIB_INCLUDE_DIR="${ZLIB_INSTALL_DIR}/include"
ZLIB_LIBRARY="${ZLIB_INSTALL_DIR}/lib/libz.a"
PNG_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/libpng-install"
PNG_INCLUDE_DIR="${PNG_INSTALL_DIR}/include"
PNG_LIBRARY="${PNG_INSTALL_DIR}/lib/libpng16.a"

CMAKE_COMMAND=cmake
NUM_CORES=$(get_num_cores)

# Add LLVM to the library path and link flags
LLVM_DIR=$(find_llvm_dir)
if [ -n "$LLVM_DIR" ]; then
    export LD_LIBRARY_PATH="${LLVM_DIR}/lib:$LD_LIBRARY_PATH"
    export LDFLAGS="-L${LLVM_DIR}/lib"
fi

# Create and clean directories
make_clean_dir "${OCV_BUILD_DIR}"
make_clean_dir "${OCV_INSTALL_DIR}"

# Configure OpenCV
# Compiler and Build Configuration
CMAKE_OPTIONS=(
    -DCMAKE_BUILD_TYPE=Release                    # Build type
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON            # Export for tooling
    -DCMAKE_C_COMPILER="${CC}"                    # C compiler
    -DCMAKE_CXX_COMPILER="${CXX}"                 # C++ compiler
    -DCMAKE_LINKER="${LD}"                        # Linker
    -DCMAKE_INSTALL_PREFIX="${OCV_INSTALL_DIR}"   # Installation directory
    -DCMAKE_CXX_STANDARD=20                       # Use C++20 standard
    -DCMAKE_CXX_STANDARD_REQUIRED=ON              # Enforce C++20 standard
    -DOPENCV_EXTRA_MODULES_PATH="${OCV_CONTRIB_SRC_DIR}/modules"
    # -DLLVM_DIR="/usr/lib/llvm-19/lib/cmake/llvm"
    # -DCMAKE_PREFIX_PATH="/usr/lib/llvm-19"
)

if [[ "$(uname)" == "Darwin" ]] && [[ $(uname -m) == "arm64" ]]; then
  CMAKE_OPTIONS+=(
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.4            # OSX dev target version
    -DCMAKE_CXX_FLAGS="-mmacosx-version-min=14.4" # Min macOS version for C++
    -DCMAKE_C_FLAGS="-mmacosx-version-min=14.4"   # Min macOS version for C
  )
fi

# Build Options
BUILD_LIST="core,imgcodecs,imgproc,video"
CMAKE_OPTIONS+=(
    -DBUILD_opencv_core=ON         # Build core, imgcodecs, imgproc, video
    -DBUILD_opencv_imgcodecs=ON    #
    -DBUILD_opencv_imgproc=ON      #
    -DBUILD_opencv_video=ON        #
    -DBUILD_opencv_highgui=OFF     #
    -DBUILD_opencv_contrib=OFF     #
    -DBUILD_opencv_features2d=OFF  #
    -DBUILD_opencv_calib3d=OFF     #
    -DBUILD_LIST="${BUILD_LIST}"   #
    -DBUILD_PROTOBUF=OFF           # Disable protobuf
    -DBUILD_DOCS=OFF               # Disable documentation build
    -DBUILD_EXAMPLES=OFF           # Disable examples build
    -DBUILD_TESTS=OFF              # Disable tests build
    -DBUILD_SHARED_LIBS=OFF        # Build static libraries
    -DBUILD_NEW_PYTHON_SUPPORT=OFF # Disable new Python support
    -DBUILD_WITH_DEBUG_INFO=OFF    # Disable debug info
    -DBUILD_PERF_TESTS=OFF         # Disable performance tests
)

# Third-party Libraries
CMAKE_OPTIONS+=(
    -DBUILD_ZLIB=OFF                           # Use custom build zlib-ng
    -DZLIB_INCLUDE_DIR="${ZLIB_INCLUDE_DIR}"   #
    -DZLIB_LIBRARY="${ZLIB_LIBRARY}"           #
    -DHAVE_PNG=ON                              # Use custom build libpng
    -DBUILD_PNG=OFF                            #
    -DPNG_PNG_INCLUDE_DIR="${PNG_INCLUDE_DIR}" #
    -DPNG_INCLUDE_DIR="${PNG_INCLUDE_DIR}"     #
    -DPNG_LIBRARY="${PNG_LIBRARY}"             #
    -DWITH_CUDA=OFF                            # NO CUDA support
    -DWITH_FFMPEG=OFF                          # NO FFMPEG support
    -DWITH_MSMF=OFF                            # NO MS Media Foundation support
    -DWITH_IPP=OFF                             # NO IPP support
    -DWITH_JPEG=ON                             # jpeg-turbo for JPEG support
    -DWITH_JPEG_TURBO=ON                       #
    -DBUILD_JPEG=OFF                           #
    -DBUILD_JPEG_TURBO=ON                      #
    -DBUILD_JPEG_TURBO_DISABLE=OFF             #
    -DWITH_OPENJPEG=ON                         # OpenJPEG for JPEG 2000 support
    -DBUILD_OPENJPEG=ON                        #
    -DWITH_TIFF=ON                             # Support TIFF build from source
    -DBUILD_TIFF=ON                            #
    -DWITH_WEBP=ON                             # Support WebP build from source
    -DBUILD_WEBP=ON                            #
    -DWITH_OPENEXR=ON                          # Support OpenEXR build from source
    -DBUILD_OPENEXR=ON                         #
    -DWITH_JASPER=OFF                          # NO Jasper support. ref: OpenCV
    -DBUILD_JASPER=OFF                         #    config options reference
    -DBUILD_WITH_STATIC_CRT=OFF                # NO static CRT
    -DBUILD_WITH_ADE=OFF                       # NO Ade support
    -DWITH_QUIRC=OFF                           # NO Quirc support
    -DBUILD_FAT_JAVA_LIB=OFF                   # NO fat Java library
)

# Source and Build Directories
CMAKE_OPTIONS+=(
    -S "${OCV_SRC_DIR}"         # Source directory
    -B "${OCV_BUILD_DIR}"       # Build directory
    # -DCMAKE_VERBOSE_MAKEFILE=ON # Verbose Makefile
    # -DCMAKE_RULE_MESSAGES=ON    # Rule messages
    # -DCMAKE_COLOR_MAKEFILE=ON   # Color makefile
)

# Print the CMake command and options
echo "Configuring OpenCV with the following options:"
echo "  " "${CMAKE_OPTIONS[@]}"
# Run CMake with the specified options
"${CMAKE_COMMAND}" "${CMAKE_OPTIONS[@]}"

# Build and install OpenCV
echo "Building OpenCV..."
"${CMAKE_COMMAND}" --build "${OCV_BUILD_DIR}" --target install \
    -- -j"${NUM_CORES}" || error_exit "Build failed."

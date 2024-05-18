#!/bin/bash

# Get the directory where the script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source common functions
source "${SCRIPT_DIR}/common_func.sh"

# Define the variables
PNG_SOURCE_DIR="${SCRIPT_DIR}/../contrib/libpng"
PNG_BUILD_DIR="${SCRIPT_DIR}/../build_contrib/libpng-build"
PNG_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/libpng-install"
ZLIB_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/zlib-ng-install"
ZLIB_INCLUDE_DIR="${ZLIB_INSTALL_DIR}/include"
ZLIB_LIBRARY="${ZLIB_INSTALL_DIR}/lib/libz.a"
CMAKE_COMMAND=cmake
NUM_CORES=$(get_num_cores)

# Create and clean directories
make_clean_dir "${PNG_BUILD_DIR}"
make_clean_dir "${PNG_INSTALL_DIR}"

# Configure zlib-ng
# Compiler and Build Configuration
CMAKE_OPTIONS=(
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.4          # OSX dev target version
    -DCMAKE_BUILD_TYPE=Release                  # Build type
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON          # Export for tooling
    -DCMAKE_C_COMPILER="${CC}"                  # C compiler
    -DCMAKE_LINKER="${LD}"                      # Linker
    -DCMAKE_INSTALL_PREFIX="${PNG_INSTALL_DIR}" # Installation directory
    -DCMAKE_C_FLAGS="-mmacosx-version-min=14.4" # Min macOS version for C
    -DCMAKE_C_STANDARD=17                       # C20 not supported, use C17
    -DCMAKE_C_STANDARD_REQUIRED=ON              #
)

# Build Options
CMAKE_OPTIONS+=(
    -DPNG_STATIC=ON                          # Build only static libraries
    -DPNG_SHARED=OFF                         #
    -DZLIB_INCLUDE_DIR="${ZLIB_INCLUDE_DIR}" # Use custom build zlib-ng
    -DZLIB_LIBRARY="${ZLIB_LIBRARY}"         #
)

# Source and Build Directories
CMAKE_OPTIONS+=(
    -S "${PNG_SOURCE_DIR}" # Source directory
    -B "${PNG_BUILD_DIR}"  # Build directory
    # -DCMAKE_VERBOSE_MAKEFILE=ON # Verbose Makefile
    # -DCMAKE_RULE_MESSAGES=ON    # Rule messages
    # -DCMAKE_COLOR_MAKEFILE=ON   # Color makefile
)

# Print the CMake command and options
echo "Configuring libpng with the following options:"
echo "  " "${CMAKE_OPTIONS[@]}"
# Run CMake with the specified options
"${CMAKE_COMMAND}" "${CMAKE_OPTIONS[@]}"

# Build and install
echo "Building libpng with ${NUM_CORES} cores..."
"${CMAKE_COMMAND}" --build "${PNG_BUILD_DIR}" --target install \
    -- -j"${NUM_CORES}" || error_exit "Build failed."

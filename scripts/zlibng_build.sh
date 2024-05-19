#!/bin/bash

# Get the directory where the script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source common functions
source "${SCRIPT_DIR}/common_func.sh"

# Define the variables
ZLIB_SOURCE_DIR="${SCRIPT_DIR}/../contrib/zlib-ng"
ZLIB_BUILD_DIR="${SCRIPT_DIR}/../build_contrib/zlib-ng-build"
ZLIB_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/zlib-ng-install"
CMAKE_COMMAND=cmake
NUM_CORES=$(get_num_cores)

# Create and clean directories
make_clean_dir "${ZLIB_BUILD_DIR}"
make_clean_dir "${ZLIB_INSTALL_DIR}"

# Configure zlib-ng
# Compiler and Build Configuration
CMAKE_OPTIONS=(
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.4           # OSX dev target version
    -DCMAKE_BUILD_TYPE=Release                   # Build type
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON           # Export for tooling
    -DCMAKE_C_COMPILER="${CC}"                   # C compiler
    -DCMAKE_LINKER="${LD}"                       # Linker
    -DCMAKE_INSTALL_PREFIX="${ZLIB_INSTALL_DIR}" # Installation directory
    -DCMAKE_C_FLAGS="-mmacosx-version-min=14.4"  # Min macOS version for C
    -DCMAKE_C_STANDARD=11                        # highest C standard to use
    -DCMAKE_C_STANDARD_REQUIRED=ON               # Enforce C++20 standard
)

# Build Options
CMAKE_OPTIONS+=(
    -DZLIB_COMPAT=ON                     # Enable zlib compatibility
    -DBUILD_SHARED_LIBS=OFF              # Build static libraries
    -DZLIB_ENABLE_TESTS=OFF              # Disable tests
    -DZLIBNG_ENABLE_TESTS=OFF            # Disable tests
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON # Position independent code
)

# Source and Build Directories (verbose)
CMAKE_OPTIONS+=(
    -S "${ZLIB_SOURCE_DIR}" # Source directory
    -B "${ZLIB_BUILD_DIR}"  # Build directory
    # -DCMAKE_VERBOSE_MAKEFILE=ON # Verbose Makefile
    # -DCMAKE_RULE_MESSAGES=ON    # Rule messages
    # -DCMAKE_COLOR_MAKEFILE=ON   # Color makefile
)

# Print the CMake command and options
echo "Configuring zlib-ng with the following options:"
echo "  " "${CMAKE_OPTIONS[@]}"
# Run CMake with the specified options
"${CMAKE_COMMAND}" "${CMAKE_OPTIONS[@]}"

# Build and install
echo "Building zlib-ng..."
"${CMAKE_COMMAND}" --build "${ZLIB_BUILD_DIR}" --target install \
    -- -j"${NUM_CORES}" || error_exit "Build failed."

#!/bin/bash

# Get the directory where the script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source common functions
source "${SCRIPT_DIR}/common_func.sh"

# Define the variables
EXIV2_BUILD_DIR="${SCRIPT_DIR}/../build_contrib/exiv2-build"
EXIV2_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/exiv2-install"
EXIV2_SRC_DIR="${SCRIPT_DIR}/../contrib/exiv2"
CMAKE_COMMAND=cmake
NUM_CORES=$(get_num_cores)

# Create and clean directories
make_clean_dir "${EXIV2_BUILD_DIR}"
make_clean_dir "${EXIV2_INSTALL_DIR}"

# Configure Exiv2
# Compiler and Build Configuration
CMAKE_OPTIONS=(
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.4            # OSX dev target version
    -DCMAKE_BUILD_TYPE=Release                    # Build type
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON            # Export for tooling
    -DCMAKE_CXX_STANDARD=17                       # Required by Exiv2
    -DCMAKE_CXX_STANDARD_REQUIRED=ON              # Enforce C++20 standard
    -DCMAKE_C_COMPILER="${CC}"                    # C compiler
    -DCMAKE_CXX_COMPILER="${CXX}"                 # C++ compiler
    -DCMAKE_LINKER="${LD}"                        # Linker
    -DCMAKE_INSTALL_PREFIX="${EXIV2_INSTALL_DIR}" # Installation directory
    -DCMAKE_CXX_FLAGS="-mmacosx-version-min=14.4" # Min macOS version for C++
    -DCMAKE_C_FLAGS="-mmacosx-version-min=14.4"   # Min macOS version for C
    -DBUILD_SHARED_LIBS=OFF                       # Build static library
    -DEXIV2_ENABLE_INIH=OFF                       # NO need to install inih
    -DEXIV2_ENABLE_BROTLI=OFF                     # NO need to install brotli
    -DEXIV2_ENABLE_CURL=OFF                       # NO network support
    -DEXIV2_ENABLE_XMP=OFF                        # NO XMP (skip expat)
    -DEXIV2_ENABLE_EXTERNAL_XMP=OFF               # NO external XMP
    -S "${EXIV2_SRC_DIR}"
    -B "${EXIV2_BUILD_DIR}"
)

# Print the CMake command and options
echo "Configuring Exiv2 with the following options:"
echo "  " "${CMAKE_OPTIONS[@]}"
# Run CMake with the specified options
"${CMAKE_COMMAND}" "${CMAKE_OPTIONS[@]}"

# Build and install Exiv2
echo "Building Exiv2..."
"${CMAKE_COMMAND}" --build "${EXIV2_BUILD_DIR}" --target install \
    -- -j"${NUM_CORES}" || error_exit "Build failed."

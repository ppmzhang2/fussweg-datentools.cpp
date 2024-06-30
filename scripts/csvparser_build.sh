#!/bin/bash

# Get the directory where the script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source common functions
. "${SCRIPT_DIR}/common_func.sh"

# Define the variables
CSV_SOURCE_DIR="${SCRIPT_DIR}/../contrib/csv-parser-cmake"
CSV_BUILD_DIR="${SCRIPT_DIR}/../build_contrib/csv-parser-build"
CSV_INSTALL_DIR="${SCRIPT_DIR}/../build_contrib/csv-parser-install"
CMAKE_COMMAND=cmake
NUM_CORES=$(get_num_cores)

# Create and clean directories
make_clean_dir "${CSV_BUILD_DIR}"
make_clean_dir "${CSV_INSTALL_DIR}"

# Configure csv-parser
# Compiler and Build Configuration
CMAKE_OPTIONS=(
    -DCMAKE_BUILD_TYPE=Release                  # Build type
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON          # Export for tooling
    -DCMAKE_C_COMPILER="${CC}"                  # C compiler
    -DCMAKE_LINKER="${LD}"                      # Linker
    -DCMAKE_INSTALL_PREFIX="${CSV_INSTALL_DIR}" # Installation directory
    -DCMAKE_CXX_STANDARD=17                     # highest C++ standard to use
    -DCMAKE_C_STANDARD_REQUIRED=ON              # Enforce C++20 standard
    -DCMAKE_CXX_STANDARD_REQUIRED=ON            # Enforce C++20 standard
)

if [[ "$(uname)" == "Darwin" ]] && [[ $(uname -m) == "arm64" ]]; then
    CMAKE_OPTIONS+=(
        -DCMAKE_OSX_DEPLOYMENT_TARGET=14.4          # OSX dev target version
        -DCMAKE_C_FLAGS="-mmacosx-version-min=14.4" # Min macOS version for C
    )
fi

# Source and Build Directories (verbose)
CMAKE_OPTIONS+=(
    -S "${CSV_SOURCE_DIR}"   # Source directory
    -B "${CSV_BUILD_DIR}"    # Build directory
    -DCSV_DEVELOPER=OFF      # Don't build the developer tools
    # -DCMAKE_VERBOSE_MAKEFILE=ON # Verbose Makefile
    # -DCMAKE_RULE_MESSAGES=ON    # Rule messages
    # -DCMAKE_COLOR_MAKEFILE=ON   # Color makefile
)

# Print the CMake command and options
echo "Configuring csv-parser with the following options:"
echo "  " "${CMAKE_OPTIONS[@]}"
# Run CMake with the specified options
"${CMAKE_COMMAND}" "${CMAKE_OPTIONS[@]}"

# Build and install
echo "Building csv-parser..."
"${CMAKE_COMMAND}" --build "${CSV_BUILD_DIR}" --config Release --target install
    #-- -j"${NUM_CORES}" || error_exit "Build failed."

#!/bin/sh

BUILD_DIR="build"
VCPKG_PATH="../vcpkg/scripts/buildsystems/vcpkg.cmake"

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Build the project
cmake -B "build/" -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_PATH" \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build "build/" --config Debug

echo "Build complete!"

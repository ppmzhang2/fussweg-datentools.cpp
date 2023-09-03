#!/bin/sh

BUILD_DIR="build"

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Build the project
cmake --preset=unix-vcpkg
cmake --build --preset=unix-vcpkg-debug

echo "Build complete!"

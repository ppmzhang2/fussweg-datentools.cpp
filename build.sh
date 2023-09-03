#!/bin/sh

BUILD_DIR="build"

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Build the project
cmake --preset=ux-vcpkg
cmake --build --preset=ux-vcpkg-debug

echo "Build complete!"

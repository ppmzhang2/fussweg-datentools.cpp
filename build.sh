#!/bin/sh

BUILD_DIR="build"

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Build the project
cmake -B "build/" -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build "build/" --config Debug # -v # for verbose output

echo "Build complete!"

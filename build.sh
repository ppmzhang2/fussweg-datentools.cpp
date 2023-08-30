#!/bin/sh

BUILD_DIR="build"

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Change into the build directory
cd "$BUILD_DIR" || exit

# Build the project
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

echo "Build complete!"

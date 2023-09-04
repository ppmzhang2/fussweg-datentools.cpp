# Fussweg Datentools

Footpath image data processing tools.

## Setting Up the Development Environment

### Using the vcpkg submodule

The project now utilizes `vcpkg` as a submodule to manage dependencies.
To ensure a smooth development process, follow these steps:

1. **Clone the repository with submodules**:

   If you're cloning the project for the first time, use the following command
   to fetch the repository along with the `vcpkg` submodule:

   ```bash
   git clone --recurse-submodules https://github.com/ppmzhang2/fussweg-datentools.cpp
   ```

   If you've already cloned the project and just want to update or initialize
   the vcpkg submodule, navigate to the project's root directory and run:

   ```bash
   git submodule update --init --recursive
   ```

2. **Bootstrap `vcpkg`**:

   Navigate to the `vcpkg` directory and run the following command:

   ```bash
   ./bootstrap-vcpkg.sh
   ```

   This will build the `vcpkg` executable and place it in the `vcpkg` directory.

## References

- Demo images from [Exif Samples](https://github.com/ianare/exif-samples)

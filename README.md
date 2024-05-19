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

   Run the following command from the project's root directory:

   ```bash
   ./vcpkg/bootstrap-vcpkg.sh
   ```

   This will build the `vcpkg` executable and place it in the `vcpkg` directory.

## Usage

### Perspective Transformation

Find the perspective transformation matrix by providing the width and height,
and eight points in the source image, which will determine two vanishing points.
The program will further calculate the quadrilateral region of interest (ROI)
by joining the vanishing points with the four apexes of the source image, and
returning the intersections of the lines.

```bash
FusswegDatentools pov-roi 5568 4872 \
1404 3764 2159 1519 5033 2712 3849 1159 3710 275 3279 295 1026 3774 5187 3710
# Output:
# TL -> 2537.67, 145.667
# TR -> 2975.28, 124.631
# BR -> 5551.82, 4842.37
# BL -> 0, 4872
```

Use the output to transform images to a bird's-eye view:

```bash
FusswegDatentools pov-transform 200 1600 \
2537.67 145.667 2975.28 124.631 5551.82 4842.37 0.0 4872.0 \
path/to/image/folder \
path/to/output/folder
```

## References

- Demo images from [Exif Samples](https://github.com/ianare/exif-samples)

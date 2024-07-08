# Fussweg Datentools

[![Build and Release](https://github.com/ppmzhang2/fussweg-datentools.cpp/actions/workflows/build.yml/badge.svg)](https://github.com/ppmzhang2/fussweg-datentools.cpp/actions/workflows/build.yml)

Footpath image data processing tools.

## Setting Up the Development Environment

The project now manages all third-party dependencies using git submodules. This
includes `exiv2`, `nlohmann_json`, `googletest`, `opencv`, and the dependencies
of `opencv` (`zlib-ng` and `libpng`). All of these reside in the `contrib` folder.

### Cloning the Repository with Submodules

To fetch the repository alone, use the following command:

```bash
git clone https://github.com/ppmzhang2/fussweg-datentools.cpp
```

If you've already cloned the project and need to update or initialize the
submodules, navigate to the project's root directory and run:

```bash
git submodule update --depth 1 --recursive --init
```

### Compiling Dependencies

To compile the project, first compile the contribs. Run the following commands
sequentially:

```bash
./scripts/csvparser_build.sh
./scripts/zlibng_build.sh
./scripts/libpng_build.sh
./scripts/exiv2_build.sh
./scripts/opencv_build.sh
```

Then, compile the project using the `build.sh` script with either the `debug`
or `release` option:

```bash
./build.sh [debug|release]
```

## Modules

The program includes three main modules:

1. `fdt::exif`: For handling EXIF data, such as extracting EXIF data from images.
2. `fdt::annot`: For annotation-related tasks, such as drawing bounding boxes on
  images based on annotations and calculating pavement distress statistics
  according to annotations.
3. `fdt::cv`: For computer vision tasks, including the perspective transformation
  example and calculating optical flow differences between image pairs.

## Usage

### Perspective Transformation

Find the perspective transformation matrix by providing the width and height,
and eight points in the source image, which will determine two vanishing
points. The program will further calculate the quadrilateral region of interest
(ROI) by joining the vanishing points with the four apexes of the source image,
and returning the intersections of the lines.

```bash
fdt pov-roi 5568 4872 \
1404 3764 2159 1519 5033 2712 3849 1159 3710 275 3279 295 1026 3774 5187 3710
# Output:
# TL -> 2537.67, 145.667
# TR -> 2975.28, 124.631
# BR -> 5551.82, 4842.37
# BL -> 0, 4872
```

Use the output to transform images to a bird's-eye view:

```bash
fdt pov-transform 200 1600 \
2537.67 145.667 2975.28 124.631 5551.82 4842.37 0.0 4872.0 \
path/to/image/folder \
path/to/output/folder
```

## References

- Demo images from [Exif Samples](https://github.com/ianare/exif-samples)

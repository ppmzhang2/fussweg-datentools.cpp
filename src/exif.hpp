/*
 * Get the EXIF data from an image.
 *
 */
#pragma once
#include <exiv2/exiv2.hpp>

/*
 * A struct to hold latitude and longitude.
 */
struct Coord {
    double lat;
    double lon;
};

/*
 * This function will return a pair containing latitude and longitude, it will
 * throw an exception if:
 * - the image cannot be opened
 * - the image does not contain any Exif data
 * - the image does not contain any GPS info
 *
 *  @param path: the path to the image
 *  @return: coordiantes containing latitude and longitude
 *  @throws std::runtime_error
 */
Coord GetCoord(const std::string &path);

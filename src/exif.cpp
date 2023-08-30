#include "exif.hpp"

double convert_to_numeric(const std::string &value_str) {
    std::vector<std::string> parts;
    std::istringstream iss(value_str);
    std::string part;

    while (std::getline(iss, part, ' ')) {
        parts.push_back(part);
    }

    double degrees = std::stod(parts[0].substr(0, parts[0].find('/')));
    double minutes = std::stod(parts[1].substr(0, parts[1].find('/')));
    double seconds = std::stod(parts[2].substr(0, parts[2].find('/'))) /
                     std::stod(parts[2].substr(parts[2].find('/') + 1));

    return degrees + (minutes / 60.0) + (seconds / 3600.0);
}

Coord getCoord(const std::string &imgPath) {
    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(imgPath);
    if (!image.get()) {
        throw std::runtime_error("Failed to open image: " + imgPath);
    }

    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        throw std::runtime_error("No Exif data found in image: " + imgPath);
    }

    Exiv2::ExifKey keyLat("Exif.GPSInfo.GPSLatitude");
    Exiv2::ExifKey keyLon("Exif.GPSInfo.GPSLongitude");
    Exiv2::ExifData::iterator itLat = exifData.findKey(keyLat);
    Exiv2::ExifData::iterator itLon = exifData.findKey(keyLon);
    if (itLat == exifData.end() || itLon == exifData.end()) {
        throw std::runtime_error("No GPS info found in image: " + imgPath);
    }

    double lat_numeric = convert_to_numeric(itLat->value().toString());
    double lon_numeric = convert_to_numeric(itLon->value().toString());

    // Check if the latitude and longitude are negative
    Exiv2::ExifKey keyLatRef("Exif.GPSInfo.GPSLatitudeRef");
    Exiv2::ExifKey keyLonRef("Exif.GPSInfo.GPSLongitudeRef");
    Exiv2::ExifData::iterator itLatRef = exifData.findKey(keyLatRef);
    Exiv2::ExifData::iterator itLonRef = exifData.findKey(keyLonRef);
    if (itLatRef != exifData.end() && itLatRef->value().toString() == "S") {
        lat_numeric = -lat_numeric;
    }
    if (itLonRef != exifData.end() && itLonRef->value().toString() == "W") {
        lon_numeric = -lon_numeric;
    }

    return Coord{lat_numeric, lon_numeric};
}

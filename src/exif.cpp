#include "exif.hpp"

/*
 * Convert to a double a string of the format:
 * degrees/1 minutes/1 seconds/10000000
 */
double StrToNum(const std::string &str) {
    std::vector<std::string> parts;
    std::istringstream iss(str);
    std::string part;

    while (std::getline(iss, part, ' ')) {
        parts.push_back(part);
    }

    const double kDeg = std::stod(parts[0].substr(0, parts[0].find('/')));
    const double kMin = std::stod(parts[1].substr(0, parts[1].find('/')));
    const double kSec = std::stod(parts[2].substr(0, parts[2].find('/'))) /
                        std::stod(parts[2].substr(parts[2].find('/') + 1));

    return kDeg + (kMin / 60.0) + (kSec / 3600.0);
}

Coord GetCoord(const std::string &path) {
    const Exiv2::Image::UniquePtr ptr_img = Exiv2::ImageFactory::open(path);
    if (!ptr_img.get()) {
        throw std::runtime_error("Failed to open image: " + path);
    }

    ptr_img->readMetadata();
    Exiv2::ExifData &dat = ptr_img->exifData();
    if (dat.empty()) {
        throw std::runtime_error("No Exif data found in image: " + path);
    }

    const Exiv2::ExifKey kKeyLat("Exif.GPSInfo.GPSLatitude");
    const Exiv2::ExifKey kKeyLon("Exif.GPSInfo.GPSLongitude");
    Exiv2::ExifData::iterator it_lat = dat.findKey(kKeyLat);
    Exiv2::ExifData::iterator it_lon = dat.findKey(kKeyLon);
    if (it_lat == dat.end() || it_lon == dat.end()) {
        throw std::runtime_error("No GPS info found in image: " + path);
    }

    double lat = StrToNum(it_lat->value().toString());
    double lon = StrToNum(it_lon->value().toString());

    // Check if the latitude and longitude are negative
    const Exiv2::ExifKey kKeyLatFlag("Exif.GPSInfo.GPSLatitudeRef");
    const Exiv2::ExifKey kKeyLonFlag("Exif.GPSInfo.GPSLongitudeRef");
    Exiv2::ExifData::iterator it_lat_flag = dat.findKey(kKeyLatFlag);
    Exiv2::ExifData::iterator it_lon_flag = dat.findKey(kKeyLonFlag);
    if (it_lat_flag != dat.end() && it_lat_flag->value().toString() == "S") {
        lat = -lat;
    }
    if (it_lon_flag != dat.end() && it_lon_flag->value().toString() == "W") {
        lon = -lon;
    }

    return Coord{lat, lon};
}

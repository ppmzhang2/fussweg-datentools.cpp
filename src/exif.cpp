#include "exif.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>

// Get string value from an ExifData::iterator for a given key:
//
// - if the key is not found, return std::nullopt
// - if the key is found, return the string value with leading and trailing
//   whitespaces removed
inline Exif::OptStr get_attr_str(Exiv2::ExifData &dat, const std::string &key) {
    Exiv2::ExifData::iterator it = dat.findKey(Exiv2::ExifKey(key));
    if (it == dat.end()) {
        return std::nullopt;
    }
    std::string str = it->value().toString();
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return std::nullopt;
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

// Get the numerator from a string of the form "num/den"
// @param str: the string to convert
// @return: the numerator
inline double frac_num(const std::string &str) {
    return std::stod(str.substr(0, str.find('/')));
}

// Get the denominator from a string of the form "num/den"
// @param str: the string to convert
// @return: the denominator
inline double frac_den(const std::string &str) {
    return std::stod(str.substr(str.find('/') + 1));
}

// Compute a fraction from a string of the form "num/den"
// @param str: the string to convert
// @return: the double value
inline double frac(Exif::OptStr str) {
    if (!str || str->empty()) {
        return 0;
    }

    const double kNum = frac_num(*str);
    const double kDen = frac_den(*str);

    return kNum / kDen;
}

// Convert to a double coordinate (i.e. latitude or longitude) from a string
// of the form:
//
// - degrees/1 minutes/1 seconds/10000000
//
// then negate the result if the direction is South or West.
//
// The string is obtained from the following EXIF attributes:
//
// - Exif.GPSInfo.GPSLatitude
// - Exif.GPSInfo.GPSLongitude
//
// @param str: the optinal string to convert
// @return: the double value
// @throws std::runtime_error if the input is a null optional
inline Exif::OptDbl get_coor(const Exif::OptStr &str, const Exif::OptStr &ref) {
    if (!str) {
        return std::nullopt;
    }

    std::vector<std::string> parts;
    std::istringstream iss(*str);
    std::string part;

    while (std::getline(iss, part, ' ')) {
        parts.push_back(part);
    }

    const double kDeg = frac_num(parts[0]); // no need to divide by 1
    const double kMin = frac_num(parts[1]); // no need to divide by 1
    const double kSec = frac(parts[2]);
    const double kCoor = kDeg + (kMin / 60) + (kSec / 3600);

    if (ref && (*ref == "S" || *ref == "W")) {
        return -kCoor;
    }
    return kCoor;
}

inline Exif::OptInt to_int(const Exif::OptStr &str) {
    if (!str || str->empty()) {
        return std::nullopt;
    }
    return std::stoi(*str);
}

inline Exif::OptStr get_model(const Exif::OptStr &make,
                              const Exif::OptStr &model) {
    if (!make || !model) {
        return std::nullopt;
    }
    return *make + " " + *model;
}

// Get the circle of confusion from the camera model.
//
// Supported models:
// - GoPro Hero 11
//
// @param model: the camera model
// @return: the circle of confusion
inline Exif::OptDbl get_coc_from_model(const Exif::OptStr &model) {
    if (model && (*model).substr(0, 12) == "GoPro HERO11") {
        return 0.005; // hardcoded for now
    }
    return std::nullopt;
}

// Compute the hyperfocal distance (in meters) from the focal length (in
// mm), aperture (f-number), and circle of confusion (in mm). formula: H =
// f^2 / (n * c) + f
// @param f: the focal length
// @param n: the aperture / f-number
// @param c: the circle of confusion
// @return: the hyperfocal distance
inline double get_hyperfocal_dist(Exif::OptDbl f, Exif::OptDbl n,
                                  Exif::OptDbl c) {
    if (!f || !n || !c) {
        return 0;
    }
    return ((std::pow(*f, 2) / (*n * *c)) + *f) / 1000;
}

// Convert a date and time string to a std::tm object; return std::nullopt
// if either the date or time string is null.
// @param ymd: the date string of the form "YYYY:MM:DD"
// @param hms: the time string of the form "HH/1 MM/1 SS/1"
// @return: the std::tm object
inline Exif::OptTm get_gps_ts(const Exif::OptStr &ymd,
                              const Exif::OptStr &hms) {
    if (!ymd || !hms) {
        return std::nullopt;
    }
    std::tm ts = {};
    std::istringstream iss(*ymd + " " + *hms);
    int int_hh, int_mm, int_ss;
    char ignore;
    iss >> std::get_time(&ts, "%Y:%m:%d");
    iss >> int_hh >> ignore >> ignore >> int_mm >> ignore >> ignore >> int_ss;
    ts.tm_hour = int_hh;
    ts.tm_min = int_mm;
    // int_ss can be
    // 1. [0, 59]
    // 2. [60, +inf]
    // TODO: exiftool parses 789 as 7.89s; need to handle this case
    if (int_ss <= 59) {
        ts.tm_sec = int_ss;
    } else {
        ts.tm_sec = 0;
    }
    return ts;
}

// Open an image and return the ExifData object
// @param path: the path to the image
// @return: the ExifData object
// @throws std::runtime_error if the image cannot be opened or if no EXIF
// data is found
inline Exiv2::ExifData get_exif(const std::string &path) {
    const Exiv2::Image::UniquePtr ptr_img = Exiv2::ImageFactory::open(path);
    if (!ptr_img.get()) {
        throw std::runtime_error("Failed to open image: " + path);
    }

    ptr_img->readMetadata();
    Exiv2::ExifData dat = ptr_img->exifData();
    if (dat.empty()) {
        throw std::runtime_error("No Exif data found in image: " + path);
    }

    return dat;
}

namespace Exif {
    Exif::Attrs GetAttrs(const std::string &path) {
        Exif::Attrs attrs = Exif::DEFAULT_ATTRS;

        Exiv2::ExifData dat = get_exif(path);

        attrs.exif_ver = get_attr_str(dat, Exif::Exiv2Keys::EXIF_VER);
        attrs.model = get_model(get_attr_str(dat, Exif::Exiv2Keys::MAKE),
                                get_attr_str(dat, Exif::Exiv2Keys::MODEL));
        attrs.desc = get_attr_str(dat, Exif::Exiv2Keys::DESC);
        attrs.height = to_int(get_attr_str(dat, Exif::Exiv2Keys::HEIGHT));
        attrs.width = to_int(get_attr_str(dat, Exif::Exiv2Keys::WIDTH));
        attrs.lat = get_coor(get_attr_str(dat, Exif::Exiv2Keys::LAT),
                             get_attr_str(dat, Exif::Exiv2Keys::LAT_REF));
        attrs.lon = get_coor(get_attr_str(dat, Exif::Exiv2Keys::LON),
                             get_attr_str(dat, Exif::Exiv2Keys::LON_REF));

        attrs.altitude = frac(get_attr_str(dat, Exif::Exiv2Keys::ALTITUDE));
        attrs.ts_gps = get_gps_ts(get_attr_str(dat, Exif::Exiv2Keys::GPS_DATE),
                                  get_attr_str(dat, Exif::Exiv2Keys::GPS_TIME));
        attrs.coc = get_coc_from_model(attrs.model);
        attrs.exposure_time =
            frac(get_attr_str(dat, Exif::Exiv2Keys::EXPOSURE_TIME));
        attrs.iso = to_int(get_attr_str(dat, Exif::Exiv2Keys::ISO));
        attrs.shutter_speed =
            frac(get_attr_str(dat, Exif::Exiv2Keys::SHUTTER_SPEED));
        attrs.aperture = frac(get_attr_str(dat, Exif::Exiv2Keys::APERTURE));
        attrs.subj_dist = frac(get_attr_str(dat, Exif::Exiv2Keys::SUBJ_DIST));
        attrs.focal_length =
            frac(get_attr_str(dat, Exif::Exiv2Keys::FOCAL_LENGTH));
        attrs.hyperfocal_dist =
            get_hyperfocal_dist(attrs.focal_length, attrs.aperture, attrs.coc);

        return attrs;
    }

    void ListAllAttrs(const std::string &path) {
        Exiv2::ExifData dat = get_exif(path);

        Exiv2::ExifData::const_iterator it = dat.begin();
        for (; it != dat.end(); ++it) {
            std::cout << it->key() << ": " << it->value().toString()
                      << std::endl;
        }
    }

    void PrintAttrs(const Exif::Attrs &attrs) {
        std::cout << "Exif version: " << *attrs.exif_ver << std::endl;
        std::cout << "Model: " << *attrs.model << std::endl;
        std::cout << "Description: " << *attrs.desc << std::endl;
        std::cout << "Height: " << *attrs.height << std::endl;
        std::cout << "Width: " << *attrs.width << std::endl;
        std::cout << "Latitude: " << *attrs.lat << std::endl;
        std::cout << "Longitude: " << *attrs.lon << std::endl;
        std::cout << "Altitude: " << *attrs.altitude << std::endl;
        std::cout << "GPS timestamp: " << std::put_time(&*attrs.ts_gps, "%c %Z")
                  << std::endl;
        std::cout << "Circle of confusion: " << *attrs.coc << std::endl;
        std::cout << "Exposure time: " << *attrs.exposure_time << std::endl;
        std::cout << "ISO: " << *attrs.iso << std::endl;
        std::cout << "Shutter speed: " << *attrs.shutter_speed << std::endl;
        std::cout << "Aperture: " << *attrs.aperture << std::endl;
        std::cout << "Subject distance: " << *attrs.subj_dist << std::endl;
        std::cout << "Focal length: " << *attrs.focal_length << std::endl;
        std::cout << "Hyperfocal distance: " << *attrs.hyperfocal_dist
                  << std::endl;
    }
} // namespace Exif

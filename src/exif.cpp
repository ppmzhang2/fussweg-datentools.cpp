#include "exif.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>

using namespace fdt;

// Contains the keys of the desired Exiv2 EXIF attributes.
namespace {

    static constexpr char kK_EXIFVER[] = "Exif.Photo.ExifVersion";
    static constexpr char kK_MAKE[] = "Exif.Image.Make";
    static constexpr char kK_MODEL[] = "Exif.Image.Model";
    static constexpr char kK_DESC[] = "Exif.Image.ImageDescription";
    static constexpr char kK_HEIGHT[] = "Exif.Photo.PixelYDimension";
    static constexpr char kK_WIDTH[] = "Exif.Photo.PixelXDimension";
    static constexpr char kK_LAT[] = "Exif.GPSInfo.GPSLatitude";
    static constexpr char kK_LON[] = "Exif.GPSInfo.GPSLongitude";
    static constexpr char kK_LAT_REF[] = "Exif.GPSInfo.GPSLatitudeRef";
    static constexpr char kK_LON_REF[] = "Exif.GPSInfo.GPSLongitudeRef";
    static constexpr char kK_ALTITUDE[] = "Exif.GPSInfo.GPSAltitude";
    static constexpr char kK_GPS_DT[] = "Exif.GPSInfo.GPSDateStamp";
    static constexpr char kK_GPS_TM[] = "Exif.GPSInfo.GPSTimeStamp";
    static constexpr char kK_SUBJ_DIST[] = "Exif.Photo.SubjectDistance";
    static constexpr char kK_ISO[] = "Exif.Photo.ISOSpeedRatings";
    static constexpr char kK_APERTURE[] = "Exif.Photo.ApertureValue";
    static constexpr char kK_SHUTTER_SPEED[] = "Exif.Photo.ShutterSpeedValue";
    static constexpr char kK_EXPOSURE_TM[] = "Exif.Photo.ExposureTime";
    static constexpr char kK_FOCAL_LENGTH[] = "Exif.Photo.FocalLength";

} // namespace

// Get string value from an ExifData::iterator for a given key:
//
// - if the key is not found, return std::nullopt
// - if the key is found, return the string value with leading and trailing
//   whitespaces removed
inline exif::OptStr get_attr_str(Exiv2::ExifData &dat, const std::string &key) {
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
inline double frac(exif::OptStr str) {
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
inline exif::OptDbl get_coor(const exif::OptStr &str, const exif::OptStr &ref) {
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

inline exif::OptInt to_int(const exif::OptStr &str) {
    if (!str || str->empty()) {
        return std::nullopt;
    }
    return std::stoi(*str);
}

inline exif::OptStr get_model(const exif::OptStr &make,
                              const exif::OptStr &model) {
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
inline exif::OptDbl get_coc_from_model(const exif::OptStr &model) {
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
inline double get_hyperfocal_dist(exif::OptDbl f, exif::OptDbl n,
                                  exif::OptDbl c) {
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
inline exif::OptTm get_gps_ts(const exif::OptStr &ymd,
                              const exif::OptStr &hms) {
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

// Returns the ExifAttributes structure containing the desired EXIF attributes.
//
// Throws a runtime_error if:
// - the image can't be opened
// - the image doesn't contain Exif data
// - any other error occurs during processing
//
// Args:
//   path: The path to the image.
//
// Returns:
//   ExifAttributes containing the desired EXIF attributes.
//
// Throws:
//   std::runtime_error if any error occurs.
exif::Attrs::Attrs(const std::string &path)
    : path(""), exif_ver(std::nullopt), desc(std::nullopt), model(std::nullopt),
      height(std::nullopt), width(std::nullopt), lat(std::nullopt),
      lon(std::nullopt), altitude(std::nullopt), ts_gps(std::nullopt),
      coc(std::nullopt), subj_dist(std::nullopt), iso(std::nullopt),
      aperture(std::nullopt), shutter_speed(std::nullopt),
      exposure_time(std::nullopt), focal_length(std::nullopt),
      hyperfocal_dist(std::nullopt) {

    Exiv2::ExifData dat = get_exif(path);

    exif_ver = get_attr_str(dat, kK_EXIFVER);
    model = get_model(get_attr_str(dat, kK_MAKE), get_attr_str(dat, kK_MODEL));
    desc = get_attr_str(dat, kK_DESC);
    height = to_int(get_attr_str(dat, kK_HEIGHT));
    width = to_int(get_attr_str(dat, kK_WIDTH));
    lat = get_coor(get_attr_str(dat, kK_LAT), get_attr_str(dat, kK_LAT_REF));
    lon = get_coor(get_attr_str(dat, kK_LON), get_attr_str(dat, kK_LON_REF));

    altitude = frac(get_attr_str(dat, kK_ALTITUDE));
    ts_gps =
        get_gps_ts(get_attr_str(dat, kK_GPS_DT), get_attr_str(dat, kK_GPS_TM));
    coc = get_coc_from_model(model);
    exposure_time = frac(get_attr_str(dat, kK_EXPOSURE_TM));
    iso = to_int(get_attr_str(dat, kK_ISO));
    shutter_speed = frac(get_attr_str(dat, kK_SHUTTER_SPEED));
    aperture = frac(get_attr_str(dat, kK_APERTURE));
    subj_dist = frac(get_attr_str(dat, kK_SUBJ_DIST));
    focal_length = frac(get_attr_str(dat, kK_FOCAL_LENGTH));
    hyperfocal_dist = get_hyperfocal_dist(focal_length, aperture, coc);
}

void exif::Attrs::Print() const {
    std::cout << "Exif version: " << *exif_ver << std::endl;
    std::cout << "Model: " << *model << std::endl;
    std::cout << "Description: " << *desc << std::endl;
    std::cout << "Height: " << *height << std::endl;
    std::cout << "Width: " << *width << std::endl;
    std::cout << "Latitude: " << *lat << std::endl;
    std::cout << "Longitude: " << *lon << std::endl;
    std::cout << "Altitude: " << *altitude << std::endl;
    std::cout << "GPS timestamp: " << std::put_time(&*ts_gps, "%c %Z")
              << std::endl;
    std::cout << "Circle of confusion: " << *coc << std::endl;
    std::cout << "Exposure time: " << *exposure_time << std::endl;
    std::cout << "ISO: " << *iso << std::endl;
    std::cout << "Shutter speed: " << *shutter_speed << std::endl;
    std::cout << "Aperture: " << *aperture << std::endl;
    std::cout << "Subject distance: " << *subj_dist << std::endl;
    std::cout << "Focal length: " << *focal_length << std::endl;
    std::cout << "Hyperfocal distance: " << *hyperfocal_dist << std::endl;
}

void exif::Attrs::ListAll(const std::string &path) {
    Exiv2::ExifData dat = get_exif(path);

    Exiv2::ExifData::const_iterator it = dat.begin();
    for (; it != dat.end(); ++it) {
        std::cout << it->key() << ": " << it->value().toString() << std::endl;
    }
}

// Define a set of functions to assign values to JSON objects
// based on the type of the value
static inline void assign_to_json(nlohmann::json &j, const std::string &key,
                                  const exif::OptStr &value) {
    if (value) {
        j[key] = *value;
    }
}

static inline void assign_to_json(nlohmann::json &j, const std::string &key,
                                  const exif::OptDbl &value) {
    if (value) {
        j[key] = *value;
    }
}

static inline void assign_to_json(nlohmann::json &j, const std::string &key,
                                  const exif::OptInt &value) {
    if (value) {
        j[key] = *value;
    }
}

static inline void assign_to_json(nlohmann::json &j, const std::string &key,
                                  const exif::OptTm &value) {
    if (value) {
        char buf[100];
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &(*value));
        j[key] = std::string(buf);
    }
}

const nlohmann::json exif::Attrs::ToJson() const {
    nlohmann::json out;

    // Define a list of serialization actions
    std::vector<std::function<void()>> serActions = {
        [&]() { assign_to_json(out, "path", path); },
        [&]() { assign_to_json(out, "exif_ver", exif_ver); },
        [&]() { assign_to_json(out, "desc", desc); },
        [&]() { assign_to_json(out, "model", model); },
        [&]() { assign_to_json(out, "height", height); },
        [&]() { assign_to_json(out, "width", width); },
        [&]() { assign_to_json(out, "lat", lat); },
        [&]() { assign_to_json(out, "lon", lon); },
        [&]() { assign_to_json(out, "altitude", altitude); },
        [&]() { assign_to_json(out, "ts_gps", ts_gps); },
    };

    for (const auto &action : serActions) {
        action(); // Execute the serialization action
    }

    return out;
}

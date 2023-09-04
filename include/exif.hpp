#pragma once

#include <exiv2/exiv2.hpp>
#include <optional>

namespace Exif {

    // Using 'using' for type aliasing
    using OptStr = std::optional<std::string>;
    using OptDbl = std::optional<double>;
    using OptInt = std::optional<int>;
    using OptTm = std::optional<std::tm>;

    // This namespace contains the keys of the desired Exiv2 EXIF attributes.
    namespace Exiv2Keys {

        static constexpr char EXIF_VER[] = "Exif.Photo.ExifVersion";
        static constexpr char MAKE[] = "Exif.Image.Make";
        static constexpr char MODEL[] = "Exif.Image.Model";
        static constexpr char DESC[] = "Exif.Image.ImageDescription";
        static constexpr char HEIGHT[] = "Exif.Photo.PixelYDimension";
        static constexpr char WIDTH[] = "Exif.Photo.PixelXDimension";
        static constexpr char LAT[] = "Exif.GPSInfo.GPSLatitude";
        static constexpr char LON[] = "Exif.GPSInfo.GPSLongitude";
        static constexpr char LAT_REF[] = "Exif.GPSInfo.GPSLatitudeRef";
        static constexpr char LON_REF[] = "Exif.GPSInfo.GPSLongitudeRef";
        static constexpr char ALTITUDE[] = "Exif.GPSInfo.GPSAltitude";
        static constexpr char GPS_DATE[] = "Exif.GPSInfo.GPSDateStamp";
        static constexpr char GPS_TIME[] = "Exif.GPSInfo.GPSTimeStamp";
        static constexpr char SUBJ_DIST[] = "Exif.Photo.SubjectDistance";
        static constexpr char ISO[] = "Exif.Photo.ISOSpeedRatings";
        static constexpr char APERTURE[] = "Exif.Photo.ApertureValue";
        static constexpr char SHUTTER_SPEED[] = "Exif.Photo.ShutterSpeedValue";
        static constexpr char EXPOSURE_TIME[] = "Exif.Photo.ExposureTime";
        static constexpr char FOCAL_LENGTH[] = "Exif.Photo.FocalLength";

    } // namespace Exiv2Keys

    // This structure holds the desired EXIF attributes.
    using Attrs = struct {
        OptStr exif_ver;
        // fields of image
        OptStr desc;
        OptStr model;
        OptInt height;
        OptInt width;
        // fields of GPS
        OptDbl lat;
        OptDbl lon;
        OptDbl altitude;
        OptTm ts_gps;
        // fields of camera
        OptDbl coc;       // circle of confusion, in mm
        OptDbl subj_dist; // subject distance, in m
        OptInt iso;       // ISO speed
        OptDbl aperture;
        OptDbl shutter_speed;
        OptDbl exposure_time;
        OptDbl focal_length;    // in mm
        OptDbl hyperfocal_dist; // in m
    };

    // Initializes the ExifAttributes structure with default values.
    static const Attrs DEFAULT_ATTRS = {
        .exif_ver = std::nullopt,
        .desc = std::nullopt,
        .model = std::nullopt,
        .height = std::nullopt,
        .width = std::nullopt,
        .lat = std::nullopt,
        .lon = std::nullopt,
        .altitude = std::nullopt,
        .ts_gps = std::nullopt,
        .coc = std::nullopt,
        .subj_dist = std::nullopt,
        .iso = std::nullopt,
        .aperture = std::nullopt,
        .shutter_speed = std::nullopt,
        .exposure_time = std::nullopt,
        .focal_length = std::nullopt,
        .hyperfocal_dist = std::nullopt,
    };

    // Returns the ExifAttributes structure containing the desired EXIF
    // attributes. Throws a runtime_error if:
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
    Attrs GetAttrs(const std::string &path);

    // Prints all EXIF attributes of an image to stdout.
    //
    // Args:
    //   path: The path to the image.
    void ListAllAttrs(const std::string &path);

    // Prints all fields of the ExifAttrs structure to stdout.
    //
    // Args:
    //   attrs: The ExifAttrs object.
    void PrintAttrs(const Attrs &attrs);

} // namespace Exif

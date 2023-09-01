/*
 * Get the EXIF data from an image.
 *
 */
#pragma once
#include <exiv2/exiv2.hpp>
#include <optional>

typedef std::optional<std::string> OptStr;
typedef std::optional<double> OptDbl;
typedef std::optional<int> OptInt;
typedef std::optional<std::tm> OptTm;

/*
 * A namespace to hold the keys to the desired EXIF attributes.
 */
namespace ExifKeys {
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
} // namespace ExifKeys

/*
 * A structure to hold EXIF attributes.
 */
typedef struct {
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
} ExifAttrs;

/*
 * This function will return the ExifAttributes structure containing the desired
 * EXIF attributes. It will throw an exception if:
 * - the image cannot be opened
 * - the image does not contain any Exif data
 *
 *  @param path: the path to the image
 *  @return: ExifAttributes containing the desired EXIF attributes
 *  @throws std::runtime_error
 */
ExifAttrs GetExifAttrs(const std::string &path);

/*
 * Print of an impage all the EXIF attributes to stdout.
 * @param path: the path to the image
 */
void ListAllAttrs(const std::string &path);

/*
 * Print all the fields of the ExifAttrs structure to stdout.
 * @param attrs: the ExifAttrs object
 */
void PrintAttrs(const ExifAttrs &attrs);

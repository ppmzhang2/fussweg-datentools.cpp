#pragma once

#include <exiv2/exiv2.hpp>
#include <nlohmann/json.hpp>
#include <optional>

namespace fdt {
    namespace exif {

        // Using 'using' for type aliasing
        using OptStr = std::optional<std::string>;
        using OptDbl = std::optional<double>;
        using OptInt = std::optional<int>;
        using OptTm = std::optional<std::tm>;

        // This structure holds the desired EXIF attributes.
        struct Attrs {
            std::string path;
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

            // constructor with default values
            Attrs()
                : path(""), exif_ver(std::nullopt), desc(std::nullopt),
                  model(std::nullopt), height(std::nullopt),
                  width(std::nullopt), lat(std::nullopt), lon(std::nullopt),
                  altitude(std::nullopt), ts_gps(std::nullopt),
                  coc(std::nullopt), subj_dist(std::nullopt), iso(std::nullopt),
                  aperture(std::nullopt), shutter_speed(std::nullopt),
                  exposure_time(std::nullopt), focal_length(std::nullopt),
                  hyperfocal_dist(std::nullopt) {}

            // constructor based on GetAttrs function
            Attrs(const std::string &path);

            const nlohmann::json ToJson() const;

            void ToCsv(std::ostream &) const;

            void Print() const;

            static void ListAll(const std::string &path);
        };

        void exportCsv(const std::string &, std::ostream &);

        void exportJson(const std::string &, std::ostream &);

    } // namespace exif
} // namespace fdt

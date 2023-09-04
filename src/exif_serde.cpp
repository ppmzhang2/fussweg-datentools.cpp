#include "exif_serde.hpp"
#include <nlohmann/json.hpp>

// Define a set of functions to assign values to JSON objects
// based on the type of the value
inline void assign_to_json(nlohmann::json &j, const std::string &key,
                           const Exif::OptStr &value) {
    if (value) {
        j[key] = *value;
    }
}

inline void assign_to_json(nlohmann::json &j, const std::string &key,
                           const Exif::OptDbl &value) {
    if (value) {
        j[key] = *value;
    }
}

inline void assign_to_json(nlohmann::json &j, const std::string &key,
                           const Exif::OptInt &value) {
    if (value) {
        j[key] = *value;
    }
}

inline void assign_to_json(nlohmann::json &j, const std::string &key,
                           const Exif::OptTm &value) {
    if (value) {
        char buf[100];
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &(*value));
        j[key] = std::string(buf);
    }
}

namespace ExifSerde {

    std::string Serialize(const Exif::Attrs &attrs) {
        nlohmann::json j;

        // Define a list of serialization actions
        std::vector<std::function<void()>> serActions = {
            [&]() { assign_to_json(j, "exif_ver", attrs.exif_ver); },
            [&]() { assign_to_json(j, "desc", attrs.desc); },
            [&]() { assign_to_json(j, "model", attrs.model); },
            [&]() { assign_to_json(j, "height", attrs.height); },
            [&]() { assign_to_json(j, "width", attrs.width); },
            [&]() { assign_to_json(j, "lat", attrs.lat); },
            [&]() { assign_to_json(j, "lon", attrs.lon); },
            [&]() { assign_to_json(j, "altitude", attrs.altitude); },
            [&]() { assign_to_json(j, "ts_gps", attrs.ts_gps); },
        };

        for (const auto &action : serActions) {
            action(); // Execute the serialization action
        }

        return j.dump(); // Convert JSON object to string
    }

} // namespace ExifSerde

#include "exif_json.hpp"
#include "file_iter.hpp"
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

namespace ExifJson {

    nlohmann::json FromAttr(const Exif::Attrs &attrs) {
        nlohmann::json out;

        // Define a list of serialization actions
        std::vector<std::function<void()>> serActions = {
            [&]() { assign_to_json(out, "exif_ver", attrs.exif_ver); },
            [&]() { assign_to_json(out, "desc", attrs.desc); },
            [&]() { assign_to_json(out, "model", attrs.model); },
            [&]() { assign_to_json(out, "height", attrs.height); },
            [&]() { assign_to_json(out, "width", attrs.width); },
            [&]() { assign_to_json(out, "lat", attrs.lat); },
            [&]() { assign_to_json(out, "lon", attrs.lon); },
            [&]() { assign_to_json(out, "altitude", attrs.altitude); },
            [&]() { assign_to_json(out, "ts_gps", attrs.ts_gps); },
        };

        for (const auto &action : serActions) {
            action(); // Execute the serialization action
        }

        return out;
    }

    nlohmann::json FromFiles(const std::string &content) {
        nlohmann::json out;

        for (const auto &path : FileIter(content)) {
            const nlohmann::json j = ExifJson::FromAttr(Exif::GetAttrs(path));
            out.push_back(j);
        }

        return out;
    }

} // namespace ExifJson

#pragma once
#include "exif.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace ExifJson {
    nlohmann::json FromAttr(const Exif::Attrs &attrs);
    nlohmann::json FromFiles(const std::string &path);
} // namespace ExifJson

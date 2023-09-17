#pragma once
#include "exif.hpp"
#include <string>

namespace ExifSerde {
    std::string Serialize(const Exif::Attrs &attrs);
} // namespace ExifSerde

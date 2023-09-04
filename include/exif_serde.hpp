#pragma once
#include "exif.hpp"

namespace ExifSerde {
    std::string Serialize(const Exif::Attrs &attrs);
} // namespace ExifSerde

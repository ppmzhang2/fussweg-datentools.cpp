#pragma once
#include "exif.hpp"

namespace ExifSerializer {
    std::string Serialize(const ExifAttrs &attrs);
} // namespace ExifSerializer

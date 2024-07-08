#pragma once

#include <cmath>
#include <tuple>

#define DEG2RAD(deg) ((deg) * M_PI / 180)
#define RAD2DEG(rad) ((rad) * 180 / M_PI)

namespace fdt {

    // Coordinate Reference System
    namespace crs {
        std::tuple<double, double> ToNzgd2000(const double, const double);

        std::tuple<double, double> FromNzgd2000(const double, const double);
    } // namespace crs

} // namespace fdt

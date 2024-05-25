#pragma once

#include <cmath>
#include <tuple>

#define DEG2RAD(deg) ((deg) * M_PI / 180)
#define RAD2DEG(rad) ((rad) * 180 / M_PI)

namespace fdt {

    // Coordinate Reference System
    namespace crs {
        std::tuple<double, double> const ToNzgd2000(double, double);

        std::tuple<double, double> const FromNzgd2000(double, double);
    } // namespace crs

} // namespace fdt

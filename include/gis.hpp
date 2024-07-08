#pragma once

#include <nlohmann/json.hpp>

namespace fdt {

    // Geographic Information System
    namespace gis {

        std::string json2wkt(const nlohmann::json &);

        void Geojson2Tsv(std::istream &, std::ostream &);

    } // namespace gis

} // namespace fdt

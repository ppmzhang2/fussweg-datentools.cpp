#include <nlohmann/json.hpp>

#include "gis.hpp"

// Enclose a string with parentheses
static inline const std::string enclose(const std::string &s,
                                        const std::string prefix = "") {
    if (prefix.empty())
        return "(" + s + ")";
    return prefix + " (" + s + ")";
}

// Convert a JSON list of two numbers representing a point to a WKT string
static inline const std::string wkt_point(const nlohmann::json &js) {
    return std::to_string(js[0].get<double>()) + " " +
           std::to_string(js[1].get<double>());
}

// Convert a JSON list of coordinates representing, either a line or a ring, to
// a WKT string
static inline const std::string wkt_coords(const nlohmann::json &js) {
    const size_t n = js.size();
    std::string wkt = "";
    for (size_t i = 0; i < n; ++i) {
        wkt += wkt_point(js[i]);
        if (i < n - 1)
            wkt += ", ";
    }
    return wkt;
}

// Convert a JSON list of coordinates representing a polygon to a WKT string
static inline const std::string wkt_polygon(const nlohmann::json &js) {
    const size_t n = js.size();
    std::string wkt = "";
    // Each element in `list_coords` is an array of coords representing a ring
    for (size_t i = 0; i < n; ++i) {
        wkt += enclose(wkt_coords(js[i]));
        if (i < n - 1)
            wkt += ", ";
    }
    return wkt;
}

// Convert a JSON list of coordinates representing a multipolygon to a WKT
// string
static inline const std::string wkt_mpolygon(const nlohmann::json &js) {
    const size_t n = js.size();
    std::string wkt = "";
    // Each element in `js` is an array of polygons
    for (size_t i = 0; i < n; ++i) {
        wkt += enclose(wkt_polygon(js[i]));
        if (i < n - 1)
            wkt += ", ";
    }
    return wkt;
}

std::string fdt::gis::json2wkt(const nlohmann::json &js) {
    const std::string geom_type = js["geometry"]["shapeType"];
    const nlohmann::json &coords = js["geometry"]["coordinates"];

    if (geom_type == "Point") {
        return enclose(wkt_point(coords), "POINT");
    } else if (geom_type == "LineString") {
        return enclose(wkt_coords(coords), "LINESTRING");
    } else if (geom_type == "Polygon") {
        return enclose(wkt_polygon(coords), "POLYGON");
    } else if (geom_type == "MultiPolygon") {
        return enclose(wkt_mpolygon(coords), "MULTIPOLYGON");
    } else {
        throw std::runtime_error("Unsupported geometry type: " + geom_type);
    }
}

static inline const std::string get_str(const nlohmann::json &js,
                                        const std::string &key) {
    if (!js.contains(key))
        return "";
    if (!js[key].is_string())
        return "";
    return js[key].get<std::string>();
}

static inline double get_num(const nlohmann::json &js, const std::string &key) {
    if (!js.contains(key))
        return 0.0;
    if (!js[key].is_number())
        return 0.0;
    return js[key].get<double>();
}

void fdt::gis::Geojson2Tsv(std::istream &geojson, std::ostream &out) {
    nlohmann::json js;
    geojson >> js;
    if (!js.contains("features"))
        throw std::runtime_error("Invalid GeoJSON format: missing 'features'");

    std::string wkt, uuid, id, road, side, material;
    double start, end, length, width, area, age;
    out << "entity_id\tasset_"
           "id\troad\tside\tstart\tclose\tlength\twidth\tarea\tage\tmaterial\tw"
           "kt"
        << std::endl;

    for (const auto &j : js["features"]) {
        if (!j.contains("geometry") || !j.contains("properties")) {
            throw std::runtime_error(
                "Invalid GeoJSON format: missing 'geometry' or 'properties'");
        }
        if (!j["geometry"].contains("shapeType") ||
            !j["geometry"].contains("coordinates") ||
            !j["properties"].contains("id")) {
            throw std::runtime_error("Invalid GeoJSON feature format");
        }
        wkt = fdt::gis::json2wkt(j);
        id = j["identifier"]["AssetId"];
        uuid = j["identifier"]["EntityId"];
        road = get_str(j["properties"], "road_id");
        side = get_str(j["properties"], "side");
        start = get_num(j["properties"], "start_m");
        end = get_num(j["properties"], "end_m");
        length = get_num(j["properties"], "length_m");
        width = get_num(j["properties"], "width_m");
        area = get_num(j["properties"], "area");
        age = get_num(j["properties"], "age");
        material = get_str(j["properties"], "footpath_surf_mat");
        out << uuid << "\t" << id << "\t" << road << "\t" << side << "\t"
            << start << "\t" << end << "\t" << length << "\t" << width << "\t"
            << area << "\t" << age << "\t" << material << "\t" << wkt
            << std::endl;
    }
}

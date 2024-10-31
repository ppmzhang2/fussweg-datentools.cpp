#include <csv.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "ibox.hpp"
#include "utils.hpp"

using namespace fdt;

namespace {
    // Define the FaultType enum class
    enum class FaultType : uint8_t {
        NONE = 0,
        BUMP,
        CRACK,
        DEPRESSION,
        DISPLACEMENT,
        POTHOLE,
        UNEVEN,
        VEGETATION,
    };

    // Define the FaultLevel enum class
    enum class FaultLevel : uint8_t {
        NONE = 0,
        FAIR = 1,
        POOR = 2,
        VPOOR = 3,
    };

    static const std::unordered_map<std::string, FaultType> kMapType = {
        {"bump", FaultType::BUMP},
        {"crack", FaultType::CRACK},
        {"depression", FaultType::DEPRESSION},
        {"displacement", FaultType::DISPLACEMENT},
        {"pothole", FaultType::POTHOLE},
        {"uneven", FaultType::UNEVEN},
        {"vegetation", FaultType::VEGETATION},
    };

    static const std::unordered_map<std::string, FaultLevel> kMapLevel = {
        {"fair", FaultLevel::FAIR},
        {"poor", FaultLevel::POOR},
        {"verypoor", FaultLevel::VPOOR},
    };

    // Bounding Box-related constants
    static const cv::Scalar kTxtColor(255, 255, 255); // White
    static const cv::Scalar kLineColor(0, 255, 0);    // Green

} // namespace

// Function to set a Fault based on input type and level
static inline void set_fault(ibox::Fault &fault, FaultType type,
                             FaultLevel level) {
    uint8_t ind_type = static_cast<uint8_t>(type);
    uint8_t ind_level = static_cast<uint8_t>(level);
    fault |= static_cast<ibox::Fault>(ind_level << (2 * (ind_type - 1)));
}

static inline FaultLevel str2faultlevel(const std::string &str) {
    try {
        return kMapLevel.at(str);
    } catch (const std::out_of_range &e) {
        return FaultLevel::NONE;
    }
}

static inline FaultType str2faulttype(const std::string &str) {
    try {
        return kMapType.at(str);
    } catch (const std::out_of_range &e) {
        return FaultType::NONE;
    }
}

// Parse CSV line
//
// example CSV line:
//   G0019580.JPG,3117257,"{}",3,2,
//   "{""name"":""rect"",""x"":2744,""y"":390,""width"":86,""height"":503}",
//   "{""fault"":{""crack"":true},""condition"":{""fair"":true}}"
// task:
//   - skip comma in curly brace
//   - reduce double double-quote to single double-quote
static std::vector<std::string> parse_csv_line(const std::string &line) {
    std::vector<std::string> result;
    std::string cell;

    uint8_t braces = 0; // skip comma in curly brace
    bool quote = false; // detect double double-quote
    for (char ch : line) {
        // whether push cell to result or not, depends on comma and in_brace
        if (ch == ',' && braces == 0) {
            result.push_back(cell);
            cell.clear();
        } else if (ch == '{') {
            braces += 1;
        } else if (ch == '}') {
            braces -= 1;
        }

        // whether add ch to cell or not, depends on quote and double
        // double-quote
        if (ch == ',' && braces == 0) {
            // comma outside of curly brace; do NOT add ch
            quote = false;
            continue;
        } else if (ch == ',' && braces != 0) {
            // comma inside of curly brace; add ch
            quote = false;
            cell += ch;
        } else if (ch == '"' && !quote) {
            // do NOT add ch as we do not know if it is a single or double
            quote = true;
        } else if (ch != '"' && quote) {
            // two conditions:
            // 1. single double-quote (ch != '"' && quote):
            //    usually ch is '{' or ','
            quote = false;
            cell += ch;
        } else if (ch == '"' && quote) {
            // double double-quote; used for key or value; add one double-quote
            quote = false;
            cell += ch;
        } else {
            cell += ch;
        }
    }

    if (!cell.empty()) {
        result.push_back(cell); // Add the last cell
    }

    return result;
}

static nlohmann::json viacsv2json(std::istream &stream) {
    std::string line;
    getline(stream, line); // Read the header line

    std::vector<std::string> header = parse_csv_line(line);

    nlohmann::json jsonArray;

    while (getline(stream, line)) {
        std::vector<std::string> values = parse_csv_line(line);

        nlohmann::json j;

        for (size_t i = 0; i < header.size() && i < values.size(); ++i) {
            if (header[i] == "region_shape_attributes" ||
                header[i] == "region_attributes") {
                j[header[i]] = nlohmann::json::parse(values[i]);
            } else {
                j[header[i]] = values[i];
            }
        }

        jsonArray.push_back(j);
    }

    return jsonArray;
}

static nlohmann::json viajson2json(std::istream &stream) {
    nlohmann::json json_raw, json_array;
    try {
        stream >> json_raw;
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "Raw JSON Parse Error: " << e.what() << "\n";
        return 2;
    }

    // Loop through "_via_img_metadata"
    for (auto &[key, value] : json_raw["_via_img_metadata"].items()) {
        nlohmann::json j;

        // skip empty "regions" as no fault is detected
        if (!value["regions"].empty()) {
            for (const auto &region : value["regions"]) {
                j["filename"] = value["filename"];
                j["region_attributes"] = region["region_attributes"];
                j["region_shape_attributes"] = region["shape_attributes"];
                // most fine-grained data; no grouping
                json_array.push_back(j);
            }
        }
    }
    return json_array;
}

// Convert JSON to std::vector<ibox::Box>
//
// Example JSON:
//
// - ["region_attributes"]["condition"]: {"fair": true}
// - ["region_attributes"]["fault"]: {"crack": true}
// - ["region_shape_attributes"]:
//   {"x": 2744, "y": 390, "width": 86, "height": 503}
//
// NOTE:
//
// - Records contain missing keys; without explicit check, the program will
//   crash.
// - For each valid record, ["region_attributes"]["condition"], i.e. level, has
//   at most one key-value pair. We only process the first item and discard the
//   rest.
// - ["region_attributes"]["fault"] may contain multiple valid key-value pairs.
//   Add them all to one fault for one record.
// - For all key-value pairs of both ["region_attributes"]["condition"] and
//   ["region_attributes"]["fault"], ONLY KEYS MATTER. Valid values are always
//   true, and thus they are IGNORED.
static std::vector<ibox::Box> json2boxarr(const nlohmann::json &json) {
    std::vector<ibox::Box> boxes;
    for (auto &j : json) {

        // Skip record if
        // - any required key is missing
        // - any required field is empty
        if (!j.contains("filename") || !j.contains("region_attributes") ||
            !j.contains("region_shape_attributes")) {
            continue;
        } else if (!j["region_attributes"].contains("condition") ||
                   !j["region_attributes"].contains("fault")) {
            continue;
        } else if (j["region_attributes"]["condition"].empty() ||
                   j["region_attributes"]["fault"].empty()) {
            continue;
        }

        ibox::Fault fault = ibox::Fault::NONE;
        FaultType type = FaultType::NONE;

        // Process only the first valid item in "condition"
        auto level_it = j["region_attributes"]["condition"].items().begin();
        FaultLevel level = str2faultlevel(level_it.key());
        // Skip if the level is invalid
        if (level == FaultLevel::NONE) {
            continue;
        }

        // Loop to include all valid fault types
        for (auto &type_it : j["region_attributes"]["fault"].items()) {
            type = str2faulttype(type_it.key());
            if (type == FaultType::NONE) {
                continue;
            } else {
                set_fault(fault, type, level);
            }
        }

        ibox::Box bx;
        bx.image = j["filename"];
        bx.x = j["region_shape_attributes"].value("x", -1);
        bx.y = j["region_shape_attributes"].value("y", -1);
        bx.w = j["region_shape_attributes"].value("width", -1);
        bx.h = j["region_shape_attributes"].value("height", -1);
        bx.fault = fault;

        boxes.push_back(bx);
    }
    return boxes;
}

static std::vector<ibox::ImgBox>
boxarr2imgboxarr(const std::vector<ibox::Box> &boxes) {
    std::map<std::string, std::vector<ibox::Box>> img_map;

    // Group the boxes by their image
    for (const auto &b : boxes) {
        // skip box with no faults and conditions
        // parse b.fault as uint32_t to check if it is 0
        if (*(uint32_t *)&b.fault == 0) {
            continue;
        }
        img_map[b.image].push_back(b);
    }

    // Convert the map to a vector of Boxes
    std::vector<ibox::ImgBox> iboxes;
    for (const auto &pair : img_map) {
        ibox::ImgBox bb;
        bb.image = pair.first;
        bb.boxes = pair.second;
        iboxes.push_back(bb);
    }

    return iboxes;
}

// Parse CSV annotation file. Example CSV record (Box):
//
// - ["region_shape_attributes"]:
//   {""name"":""rect"",""x"":1730,""y"":2613,""width"":737,""height"":577}
// - ["region_attributes"]
//   {""fault"":{""bump"":true,""crack"":true},""condition"":{""poor"":true}}
#ifdef GTEST_ACCESS
std::vector<ibox::ImgBox> ibox::from_via_csv(std::istream &stream_i) {
#else
static std::vector<ibox::ImgBox> from_via_csv(std::istream &stream_i) {
#endif
    const nlohmann::json json = viacsv2json(stream_i);
    const std::vector<ibox::Box> bx_arr = json2boxarr(json);
    const std::vector<ibox::ImgBox> ibx_arr = boxarr2imgboxarr(bx_arr);
    return ibx_arr;
}

// Parse JSON annotation file. Example JSON of one image (ImgBox):
//
// ```json
// {
//   "filename": "G0021000.JPG",
//   "size": 3116288,
//   "regions": [
//     {
//       "shape_attributes": {
//         "name": "rect", "x": 1730, "y": 2613, "width": 737, "height": 577
//       },
//       "region_attributes": {
//         "fault": { "pothole": true }, "condition": { "verypoor": true }
//       }
//     },
//     {
//       "shape_attributes": {
//         "name": "rect", "x": 2211, "y": 1532, "width": 679, "height": 1014
//       },
//       "region_attributes": {
//         "fault": { "crack": true }, "condition": { "poor": true }
//       }
//     }
//   ],
//   "file_attributes": {}
// }
// ```
#ifdef GTEST_ACCESS
std::vector<ibox::ImgBox> ibox::from_via_json(std::istream &stream_i) {
#else
static std::vector<ibox::ImgBox> from_via_json(std::istream &stream_i) {
#endif
    const nlohmann::json json = viajson2json(stream_i);
    const std::vector<ibox::Box> bx_arr = json2boxarr(json);
    const std::vector<ibox::ImgBox> ibx_arr = boxarr2imgboxarr(bx_arr);
    return ibx_arr;
}

std::vector<ibox::ImgBox> ibox::fromVia(const std::string &dir) {
    std::vector<ibox::ImgBox> iboxes;
    for (const auto &f : fdt::utils::listAllFiles(dir, ".csv")) {
        std::ifstream stream(f);
        if (stream.is_open()) {
            std::vector<ibox::ImgBox> iboxes_csv = from_via_csv(stream);
            iboxes.insert(iboxes.end(), iboxes_csv.begin(), iboxes_csv.end());
        }
    }
    for (const auto &f : fdt::utils::listAllFiles(dir, ".json")) {
        std::ifstream stream(f);
        if (stream.is_open()) {
            std::vector<ibox::ImgBox> iboxes_json = from_via_json(stream);
            iboxes.insert(iboxes.end(), iboxes_json.begin(), iboxes_json.end());
        }
    }
    return iboxes;
}

#ifdef GTEST_ACCESS
std::vector<ibox::ImgBox> ibox::from_csv_reader(csv::CSVReader &reader) {
#else
static std::vector<ibox::ImgBox> from_csv_reader(csv::CSVReader &reader) {
#endif
    std::vector<ibox::Box> bx_arr;
    ibox::Box bx;

    for (const auto &row : reader) {
        const auto image = row["image"].get<std::string_view>();
        const auto level = row["level"].get<std::string>();
        const auto cate = row["category"].get<std::string>();

        bx.image = image;
        bx.fault = ibox::Fault::NONE;
        bx.x = row["x"].get<int>();
        bx.y = row["y"].get<int>();
        bx.w = row["w"].get<int>();
        bx.h = row["h"].get<int>();
        FaultType fault_type = str2faulttype(cate);
        FaultLevel fault_lvl = str2faultlevel(level);
        set_fault(bx.fault, fault_type, fault_lvl);
        bx_arr.push_back(bx);
    }

    std::vector<ibox::ImgBox> ibx_arr = boxarr2imgboxarr(bx_arr);
    return ibx_arr;
}

std::vector<ibox::ImgBox> ibox::fromTsv(const std::string &dir) {
    csv::CSVFormat format;
    format.delimiter('\t').header_row(0);

    std::vector<ibox::ImgBox> ibx_arr;
    for (const auto &f : fdt::utils::listAllFiles(dir, ".tsv")) {
        csv::CSVReader reader(f, format);
        std::vector<ibox::ImgBox> ibx_arr_tsv = from_csv_reader(reader);
        ibx_arr.insert(ibx_arr.end(), ibx_arr_tsv.begin(), ibx_arr_tsv.end());
    }
    return ibx_arr;
}

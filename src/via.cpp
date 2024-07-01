#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "utils.hpp"
#include "via.hpp"

using namespace fdt;

namespace {
    // Output TSV header
    inline static constexpr const char *kTsvHeader =
        "prefix\timage\tcate\tlevel\tx\ty\tw\th";

    // Fault-related constants
    inline static constexpr uint8_t kNFaultLevel = 3;
    inline static constexpr std::array<const char *, kNFaultLevel + 1>
        kArrLevelStr = {"", "fair", "poor", "verypoor"};
    inline static constexpr uint8_t kNFaultType = 7;
    inline static constexpr std::array<const char *, kNFaultType> kArrTypeStr =
        {"bump",       "crack",  "depression", "displacement",
         "vegetation", "uneven", "pothole"};
    inline static constexpr uint8_t kNFault = 21;

    // Define the FaultType enum class
    enum class FaultType : uint8_t {
        NONE = 0,
        BUMP,
        CRACK,
        DEPRESSION,
        DISPLACEMENT,
        VEGETATION,
        UNEVEN,
        POTHOLE,
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
        {"vegetation", FaultType::VEGETATION},
        {"uneven", FaultType::UNEVEN},
        {"pothole", FaultType::POTHOLE},
    };

    static const std::unordered_map<std::string, FaultLevel> kMapLevel = {
        {"fair", FaultLevel::FAIR},
        {"poor", FaultLevel::POOR},
        {"verypoor", FaultLevel::VPOOR},
    };

    // Bounding Box-related constants
    static constexpr int kThick = 5;
    static constexpr int kFontFace = cv::FONT_HERSHEY_SIMPLEX;
    static constexpr double kFontScale = 2.5;
    static const cv::Scalar kTxtColor(255, 255, 255); // White
    static const cv::Scalar kLineColor(0, 255, 0);    // Green

} // namespace

// Function to set a Fault based on input type and level
static inline void set_fault(via::Fault &fault, FaultType type,
                             FaultLevel level) {
    uint8_t ind_type = static_cast<uint8_t>(type);
    uint8_t ind_level = static_cast<uint8_t>(level);
    fault |= static_cast<via::Fault>(ind_level << (2 * (ind_type - 1)));
}

// Type-wise MAX operator for Fault enum class to get the maximum level of
// distress
static inline void max_fault(via::Fault &lhs, via::Fault rhs) {
    for (int i = 0; i < kNFaultType; i++) {
        uint8_t cond_lhs = (static_cast<uint16_t>(lhs) >> (i * 2)) & 0b11;
        uint8_t cond_rhs = (static_cast<uint16_t>(rhs) >> (i * 2)) & 0b11;
        uint8_t cond = MAX2(cond_lhs, cond_rhs);
        lhs = static_cast<via::Fault>(
            (static_cast<uint16_t>(lhs) & ~(0b11 << (i * 2))) |
            (cond << (i * 2)));
    }
}

static inline constexpr FaultLevel str2faultlevel(const std::string &str) {
    try {
        return kMapLevel.at(str);
    } catch (const std::out_of_range &e) {
        return FaultLevel::NONE;
    }
}

static inline constexpr FaultType str2faulttype(const std::string &str) {
    try {
        return kMapType.at(str);
    } catch (const std::out_of_range &e) {
        return FaultType::NONE;
    }
}

static inline const std::string faulttype2str(uint8_t type) {
    try {
        return kArrTypeStr.at(type);
    } catch (const std::out_of_range &e) {
        return "";
    }
}

// Convert distress level to string
static inline const std::string faultlevel2str(uint8_t level) {
    try {
        return kArrLevelStr.at(level);
    } catch (const std::out_of_range &e) {
        return "";
    }
}

// Convert Fault to string
const std::string via::fault2str(via::Fault fault) {
    std::string res = "";
    for (int ind_type = 0; ind_type < kNFaultType; ind_type++) {
        uint8_t ind_level =
            (static_cast<uint16_t>(fault) >> (ind_type * 2)) & 0b11;
        if (ind_level != 0) {
            if (!res.empty())
                res += "_";
            res += faulttype2str(ind_type) + "_" + faultlevel2str(ind_level);
        }
    }
    return res;
}

static inline std::array<uint8_t, kNFault> fault2count(via::Fault fault) {
    std::array<uint8_t, kNFault> arr = {0};
    for (int idx_type = 0; idx_type < kNFaultType; ++idx_type) {
        uint8_t idx_level =
            (static_cast<uint16_t>(fault) >> (idx_type * 2)) & 0b11;
        if (idx_level != 0) {
            int idx = idx_type * kNFaultLevel + (idx_level - 1);
            arr[idx] = 1;
        }
    }
    return arr;
}

// constructor
via::ImgFaultCount::ImgFaultCount()
    : image(""), bump_fair(0), bump_poor(0), bump_verypoor(0), crack_fair(0),
      crack_poor(0), crack_verypoor(0), depression_fair(0), depression_poor(0),
      depression_verypoor(0), displacement_fair(0), displacement_poor(0),
      displacement_verypoor(0), vegetation_fair(0), vegetation_poor(0),
      vegetation_verypoor(0), uneven_fair(0), uneven_poor(0),
      uneven_verypoor(0), pothole_fair(0), pothole_poor(0),
      pothole_verypoor(0) {}

// constructor with ImgBox
via::ImgFaultCount::ImgFaultCount(const ImgBox &ibox)
    : image(ibox.image), bump_fair(0), bump_poor(0), bump_verypoor(0),
      crack_fair(0), crack_poor(0), crack_verypoor(0), depression_fair(0),
      depression_poor(0), depression_verypoor(0), displacement_fair(0),
      displacement_poor(0), displacement_verypoor(0), vegetation_fair(0),
      vegetation_poor(0), vegetation_verypoor(0), uneven_fair(0),
      uneven_poor(0), uneven_verypoor(0), pothole_fair(0), pothole_poor(0),
      pothole_verypoor(0) {
    for (const auto &box : ibox.boxes) {
        std::array<uint8_t, kNFault> arr = fault2count(box.fault);
        bump_fair += arr[0];
        bump_poor += arr[1];
        bump_verypoor += arr[2];
        crack_fair += arr[3];
        crack_poor += arr[4];
        crack_verypoor += arr[5];
        depression_fair += arr[6];
        depression_poor += arr[7];
        depression_verypoor += arr[8];
        displacement_fair += arr[9];
        displacement_poor += arr[10];
        displacement_verypoor += arr[11];
        vegetation_fair += arr[12];
        vegetation_poor += arr[13];
        vegetation_verypoor += arr[14];
        uneven_fair += arr[15];
        uneven_poor += arr[16];
        uneven_verypoor += arr[17];
        pothole_fair += arr[18];
        pothole_poor += arr[19];
        pothole_verypoor += arr[20];
    }
}

const std::string via::ImgFaultCount::ToStr() const {
    return image + "," + std::to_string(bump_fair) + "," +
           std::to_string(bump_poor) + "," + std::to_string(bump_verypoor) +
           "," + std::to_string(crack_fair) + "," + std::to_string(crack_poor) +
           "," + std::to_string(crack_verypoor) + "," +
           std::to_string(depression_fair) + "," +
           std::to_string(depression_poor) + "," +
           std::to_string(depression_verypoor) + "," +
           std::to_string(displacement_fair) + "," +
           std::to_string(displacement_poor) + "," +
           std::to_string(displacement_verypoor) + "," +
           std::to_string(vegetation_fair) + "," +
           std::to_string(vegetation_poor) + "," +
           std::to_string(vegetation_verypoor) + "," +
           std::to_string(uneven_fair) + "," + std::to_string(uneven_poor) +
           "," + std::to_string(uneven_verypoor) + "," +
           std::to_string(pothole_fair) + "," + std::to_string(pothole_poor) +
           "," + std::to_string(pothole_verypoor);
}

void via::FaultStats::AddFault(const Fault &f) {
    std::array<uint8_t, kNFault> arr = fault2count(f);
    bump_fair += arr[0];
    bump_poor += arr[1];
    bump_verypoor += arr[2];
    crack_fair += arr[3];
    crack_poor += arr[4];
    crack_verypoor += arr[5];
    depression_fair += arr[6];
    depression_poor += arr[7];
    depression_verypoor += arr[8];
    displacement_fair += arr[9];
    displacement_poor += arr[10];
    displacement_verypoor += arr[11];
    vegetation_fair += arr[12];
    vegetation_poor += arr[13];
    vegetation_verypoor += arr[14];
    uneven_fair += arr[15];
    uneven_poor += arr[16];
    uneven_verypoor += arr[17];
    pothole_fair += arr[18];
    pothole_poor += arr[19];
    pothole_verypoor += arr[20];
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

static nlohmann::json filecsv2json(std::istream &stream) {
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

static nlohmann::json filejson2json(std::istream &stream) {
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

// Convert JSON to BoxArr
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
static via::BoxArr json2boxarr(const nlohmann::json &json) {
    via::BoxArr boxes;
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

        via::Fault fault = via::Fault::NONE;
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

        via::Box bx;
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

static via::ImgBoxArr boxarr2imgboxarr(const via::BoxArr &boxes) {
    std::map<std::string, via::BoxArr> img_map;

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
    via::ImgBoxArr iboxes;
    for (const auto &pair : img_map) {
        via::ImgBox bb;
        bb.image = pair.first;
        bb.boxes = pair.second;
        iboxes.push_back(bb);
    }

    return iboxes;
}

// Convert Box to TSV strings. Format:
//
//  prefix,image,cate,level,x,y,w,h
const std::string via::Box::ToTsv(const std::string &group) const {
    std::string res = "";
    for (int ind_type = 0; ind_type < kNFaultType; ind_type++) {
        uint8_t ind_level =
            (static_cast<uint16_t>(fault) >> (ind_type * 2)) & 0b11;
        if (ind_level != 0) {
            res += group + "\t" + image + "\t" + faulttype2str(ind_type) +
                   "\t" + faultlevel2str(ind_level) + "\t" + std::to_string(x) +
                   "\t" + std::to_string(y) + "\t" + std::to_string(w) + "\t" +
                   std::to_string(h) + "\n";
        }
    }
    return res;
}

const std::string via::ImgBox::ToTsv(const std::string &group) const {
    std::string res = "";
    for (const auto &box : boxes) {
        res += box.ToTsv(group);
    }
    return res;
}

// Parse CSV annotation file. Example CSV record (Box):
//
// - ["region_shape_attributes"]:
//   {""name"":""rect"",""x"":1730,""y"":2613,""width"":737,""height"":577}
// - ["region_attributes"]
//   {""fault"":{""bump"":true,""crack"":true},""condition"":{""poor"":true}}
via::ImgBoxArr via::parseCsv(std::istream &stream_i) {
    const nlohmann::json json = filecsv2json(stream_i);
    const BoxArr bx_arr = json2boxarr(json);
    const ImgBoxArr ibx_arr = boxarr2imgboxarr(bx_arr);
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
via::ImgBoxArr via::parseJson(std::istream &stream_i) {
    const nlohmann::json json = filejson2json(stream_i);
    const BoxArr bx_arr = json2boxarr(json);
    const ImgBoxArr ibx_arr = boxarr2imgboxarr(bx_arr);
    return ibx_arr;
}

void via::drawImgBox(const ImgBox &bbx, const std::string &src,
                     const std::string &dst) {

    Fault fault_img = Fault::NONE;
    std::filesystem::path dir_src(src);
    std::filesystem::path dir_dst(dst);

    // Step 1: Read the Image
    cv::Mat img = cv::imread(dir_src / bbx.image, cv::IMREAD_COLOR);
    if (img.empty()) { // Check if the image is loaded
        std::cerr << "Error: Image " << bbx.image << " cannot be loaded!"
                  << std::endl;
        return;
    }

    // Step 2: Draw the Bounding Boxes
    for (const auto &bbx : bbx.boxes) {
        // get all faults for categorizing
        max_fault(fault_img, bbx.fault);

        cv::Rect box(bbx.x, bbx.y, bbx.w, bbx.h);
        // Positioning the text above the box
        cv::Point txtOrg(box.x, box.y - 10);
        cv::rectangle(img, box, kLineColor, kThick);

        // Add text
        std::string txt = fault2str(bbx.fault);
        cv::putText(img, txt, txtOrg, kFontFace, kFontScale, kTxtColor, kThick);
    }

    // Step 3. Save the Image
    if (!cv::imwrite(dir_dst / (fault2str(fault_img) + "_" + bbx.image), img)) {
        std::cerr << "Failed to save image" << std::endl;
    }
}

void via::drawImgBoxes(const std::string &dir_lab, const std::string &src,
                       const std::string &dst, const std::string &ext) {
    ImgBoxArr iboxes;
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ext)) {
        std::ifstream stream_if(f);
        if (!stream_if.is_open()) {
            throw std::runtime_error("Error: Could not open file " + f);
        }
        // TODO: make it more elegant
        if (ext == ".csv") {
            iboxes = via::parseCsv(stream_if);
        } else {
            iboxes = via::parseJson(stream_if);
        }
        for (auto &ibox : iboxes) {
            via::drawImgBox(ibox, src, dst);
        }
    }
}

void via::exportTsv(const std::string &dir_lab, const std::string &group,
                    std::ostream &stream_o) {
    // Write the header
    stream_o << kTsvHeader << std::endl;
    // Loop through all CSV files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".csv")) {
        std::ifstream stream_if(f);
        ImgBoxArr ibx_arr = via::parseCsv(stream_if);
        for (const auto &ibx : ibx_arr) {
            stream_o << ibx.ToTsv(group);
        }
    }
    // Loop through all JSON files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".json")) {
        std::ifstream stream_if(f);
        ImgBoxArr ibx_arr = via::parseJson(stream_if);
        for (const auto &ibx : ibx_arr) {
            stream_o << ibx.ToTsv(group);
        }
    }
}

void via::exportStats(const std::string &dir_lab, const std::string &filename) {
    // Create an empty file
    std::ofstream file(filename, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    // Write the header
    file
        << "image,bump_fair,bump_poor,bump_verypoor,crack_fair,crack_poor,"
           "crack_verypoor,depression_fair,depression_poor,depression_verypoor,"
           "displacement_fair,displacement_poor,displacement_verypoor,"
           "vegetation_fair,vegetation_poor,vegetation_verypoor,"
           "uneven_fair,uneven_poor,uneven_verypoor,"
           "pothole_fair,pothole_poor,pothole_verypoor\n";

    // Loop through all CSV files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".csv")) {
        std::ifstream stream_if(f);
        ImgBoxArr ibx_arr = via::parseCsv(stream_if);
        for (const auto &ibx : ibx_arr) {
            ImgFaultCount fc = ImgFaultCount(ibx);
            file << fc.ToStr() << "\n";
        }
    }
    // Loop through all JSON files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".json")) {
        std::ifstream stream_if(f);
        ImgBoxArr ibx_arr = via::parseJson(stream_if);
        for (const auto &ibx : ibx_arr) {
            ImgFaultCount fc = ImgFaultCount(ibx);
            file << fc.ToStr() << "\n";
        }
    }
}

void via::printStats(const std::string &dir_lab) {
    FaultStats stats;
    unsigned int img_cnt = 0;
    // Loop through all CSV files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".csv")) {
        std::ifstream stream_if(f);
        ImgBoxArr ibx_arr = parseCsv(stream_if);
        for (auto &ibx : ibx_arr) {
            for (auto &bx : ibx.boxes) {
                stats.AddFault(bx.fault);
            }
        }
        img_cnt += ibx_arr.size();
    }
    // Loop through all JSON files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".json")) {
        std::ifstream stream_if(f);
        ImgBoxArr ibx_arr = parseJson(stream_if);
        for (auto &ibx : ibx_arr) {
            for (auto &bx : ibx.boxes) {
                stats.AddFault(bx.fault);
            }
        }
        img_cnt += ibx_arr.size();
    }
    std::cout << "Total images: " << img_cnt << std::endl;
    stats.Print();
}

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "annot.hpp"
#include "utils.hpp"

using namespace fdt;

namespace {
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
static inline void set_fault(annot::Fault &fault, FaultType type,
                             FaultLevel level) {
    uint8_t ind_type = static_cast<uint8_t>(type);
    uint8_t ind_level = static_cast<uint8_t>(level);
    fault |= static_cast<annot::Fault>(ind_level << (2 * (ind_type - 1)));
}

// Type-wise OR operator for Fault enum class to get the maximum level of
// distress
// TODO: revision and test
static inline annot::Fault or_fault(annot::Fault lhs, annot::Fault rhs) {
    uint16_t result = 0;
    for (int i = 0; i < kNFaultType; i++) {
        uint8_t cond_lhs = (static_cast<uint16_t>(lhs) >> (i * 2)) & 0b11;
        uint8_t cond_rhs = (static_cast<uint16_t>(rhs) >> (i * 2)) & 0b11;
        uint8_t cond = MAX2(cond_lhs, cond_rhs);
        result |= (cond << (i * 2));
    }
    return static_cast<annot::Fault>(result);
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
const std::string annot::fault2str(annot::Fault fault) {
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

static inline std::array<uint8_t, kNFault> fault2count(annot::Fault fault) {
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
annot::ImgFaultCount::ImgFaultCount()
    : image(""), bump_fair(0), bump_poor(0), bump_verypoor(0), crack_fair(0),
      crack_poor(0), crack_verypoor(0), depression_fair(0), depression_poor(0),
      depression_verypoor(0), displacement_fair(0), displacement_poor(0),
      displacement_verypoor(0), vegetation_fair(0), vegetation_poor(0),
      vegetation_verypoor(0), uneven_fair(0), uneven_poor(0),
      uneven_verypoor(0), pothole_fair(0), pothole_poor(0),
      pothole_verypoor(0) {}

// constructor with ImgBox
annot::ImgFaultCount::ImgFaultCount(const ImgBox &ibox)
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

const std::string annot::ImgFaultCount::ToStr() const {
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

void annot::FaultStats::AddFault(const Fault &f) {
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

static nlohmann::json read_csv(std::istream &stream) {
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

static nlohmann::json read_json(std::istream &stream) {
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

static annot::BoxArr json_to_box(const nlohmann::json &json) {
    annot::BoxArr boxes;
    for (auto &j : json) {
        FaultLevel cond = FaultLevel::NONE;
        FaultType type = FaultType::NONE;
        annot::Fault fault = annot::Fault::NONE;

        // TODO: check if the key exists
        for (auto &c : j["region_attributes"]["condition"].items()) {
            try {
                cond = kMapLevel.at(c.key());
            } catch (const std::out_of_range &e) {
                continue;
            }
        }

        // TODO: check if the key exists
        for (auto &f : j["region_attributes"]["fault"].items()) {
            if (f.value() == false) {
                continue;
            } else {
                try {
                    type = kMapType.at(f.key());
                    set_fault(fault, type, cond);
                } catch (const std::out_of_range &e) {
                    continue;
                }
            }
        }

        if (fault == annot::Fault::NONE) {
            continue;
        }

        annot::Box bx;
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

static annot::ImgBoxArr group_box(const annot::BoxArr &boxes) {
    std::map<std::string, annot::BoxArr> img_map;

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
    annot::ImgBoxArr iboxes;
    for (const auto &pair : img_map) {
        annot::ImgBox bb;
        bb.image = pair.first;
        bb.boxes = pair.second;
        iboxes.push_back(bb);
    }

    return iboxes;
}

annot::ImgBoxArr annot::parseCsv(const std::string &path) {
    std::ifstream file(path);
    nlohmann::json json = read_csv(file);
    ImgBoxArr iboxes = group_box(json_to_box(json));
    return iboxes;
}

annot::ImgBoxArr annot::parseJson(const std::string &path) {
    std::ifstream file(path);
    nlohmann::json json = read_json(file);
    BoxArr boxes = json_to_box(json);
    ImgBoxArr iboxes = group_box(boxes);
    return iboxes;
}

void annot::drawImgBox(const ImgBox &bbx, const std::string &src,
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
        fault_img = or_fault(fault_img, bbx.fault);

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

void annot::drawImgBoxes(const std::string &dir_lab, const std::string &src,
                         const std::string &dst, const std::string &ext) {
    ImgBoxArr iboxes;
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ext)) {
        // TODO: make it more elegant
        if (ext == ".csv") {
            iboxes = annot::parseCsv(f);
        } else {
            iboxes = annot::parseJson(f);
        }
        for (auto &ibox : iboxes) {
            annot::drawImgBox(ibox, src, dst);
        }
    }
}

void annot::exportStats(const std::string &dir_lab,
                        const std::string &filename) {
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
        ImgBoxArr iboxes = annot::parseCsv(f);
        for (const auto &ibox : iboxes) {
            ImgFaultCount fc = ImgFaultCount(ibox);
            file << fc.ToStr() << "\n";
        }
    }
    // Loop through all JSON files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".json")) {
        ImgBoxArr iboxes = annot::parseJson(f);
        for (const auto &ibox : iboxes) {
            ImgFaultCount fc = ImgFaultCount(ibox);
            file << fc.ToStr() << "\n";
        }
    }
}

void annot::printStats(const std::string &dir_lab) {
    FaultStats stats;
    unsigned int img_cnt = 0;
    // Loop through all CSV files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".csv")) {
        ImgBoxArr iboxes = parseCsv(f);
        for (auto &ibox : iboxes) {
            for (auto &bx : ibox.boxes) {
                stats.AddFault(bx.fault);
            }
        }
        img_cnt += iboxes.size();
    }
    // Loop through all JSON files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".json")) {
        ImgBoxArr iboxes = parseJson(f);
        for (auto &ibox : iboxes) {
            for (auto &bx : ibox.boxes) {
                stats.AddFault(bx.fault);
            }
        }
        img_cnt += iboxes.size();
    }
    std::cout << "Total images: " << img_cnt << std::endl;
    stats.Print();
}

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "annot.hpp"
#include "utils.hpp"

using namespace fdt;

namespace {
    // thickness of the bounding box
    static constexpr int kThick = 5;
    static constexpr int kFontFace = cv::FONT_HERSHEY_SIMPLEX;
    static constexpr double kFontScale = 2.5;
    static const cv::Scalar kTxtColor(255, 255, 255); // White
    static const cv::Scalar kLineColor(0, 255, 0);    // Green
} // namespace

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
        unsigned int cond = 0;
        annot::Fault fault;

        // TODO: check if the key exists
        for (auto &c : j["region_attributes"]["condition"].items()) {
            if (c.key() == "fair") {
                cond = 1;
            } else if (c.key() == "poor") {
                cond = 2;
            } else if (c.key() == "verypoor") {
                cond = 3;
            }
        }

        // TODO: check if the key exists
        for (auto &f : j["region_attributes"]["fault"].items()) {
            if (f.value() == false) {
                continue;
            }
            if (f.key() == "bump") {
                fault.bump = cond;
            } else if (f.key() == "crack") {
                fault.crack = cond;
            } else if (f.key() == "depression") {
                fault.depression = cond;
            } else if (f.key() == "displacement") {
                fault.displacement = cond;
            } else if (f.key() == "vegetation") {
                fault.vegetation = cond;
            } else if (f.key() == "uneven") {
                fault.uneven = cond;
            } else if (f.key() == "pothole") {
                fault.pothole = cond;
            }
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

inline static annot::Fault combine(const annot::Fault &lhs,
                                   const annot::Fault &rhs) {
    annot::Fault c;
    c.bump = MAX2(lhs.bump, rhs.bump);
    c.crack = MAX2(lhs.crack, rhs.crack);
    c.depression = MAX2(lhs.depression, rhs.depression);
    c.displacement = MAX2(lhs.displacement, rhs.displacement);
    c.vegetation = MAX2(lhs.vegetation, rhs.vegetation);
    c.uneven = MAX2(lhs.uneven, rhs.uneven);
    c.pothole = MAX2(lhs.pothole, rhs.pothole);
    return c;
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

    Fault fault_all; // for categorizing
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
        fault_all = combine(fault_all, bbx.fault);

        cv::Rect box(bbx.x, bbx.y, bbx.w, bbx.h);
        // Positioning the text above the box
        cv::Point txtOrg(box.x, box.y - 10);
        cv::rectangle(img, box, kLineColor, kThick);

        // Add text
        std::string txt = bbx.fault.ToStr();
        cv::putText(img, txt, txtOrg, kFontFace, kFontScale, kTxtColor, kThick);
    }

    // Step 3. Save the Image
    if (!cv::imwrite(dir_dst / (fault_all.ToStr() + "_" + bbx.image), img)) {
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
            FaultCount fc;
            fc.FromBBox(ibox);
            file << fc.ToStr() << "\n";
        }
    }
    // Loop through all JSON files
    for (const auto &f : fdt::utils::listAllFiles(dir_lab, ".json")) {
        ImgBoxArr iboxes = annot::parseJson(f);
        for (const auto &ibox : iboxes) {
            FaultCount fc;
            fc.FromBBox(ibox);
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

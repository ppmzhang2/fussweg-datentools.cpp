#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "annot.hpp"
#include "path_finder.hpp"

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

static nlohmann::json csv_to_json(std::istream &stream) {
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

static std::vector<Box> json_to_box(const nlohmann::json &json) {
    std::vector<Box> boxes;
    for (auto &j : json) {
        unsigned int cond = 0;
        Fault fault;
        std::array<bool, 3> conditions = {false};

        for (auto &c : j["region_attributes"]["condition"].items()) {
            if (c.key() == "fair") {
                cond = 1;
            } else if (c.key() == "poor") {
                cond = 2;
            } else if (c.key() == "verypoor") {
                cond = 3;
            }
        }

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

        Box bx;
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

static std::vector<BBox> group_box(const std::vector<Box> &boxes) {
    std::map<std::string, std::vector<Box>> img_map;

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
    std::vector<BBox> bboxes;
    for (const auto &pair : img_map) {
        BBox bb;
        bb.image = pair.first;
        bb.boxes = pair.second;
        bboxes.push_back(bb);
    }

    return bboxes;
}

static std::string join(const std::vector<std::string> &vec,
                        const std::string &dlm) {
    std::string res;
    for (size_t i = 0; i < vec.size(); ++i) {
        res += vec[i];
        if (i < vec.size() - 1) {
            res += dlm;
        }
    }
    return res;
}

Fault combine(const Fault &lhs, const Fault &rhs) {
    Fault c;
    c.bump = lhs.bump | rhs.bump;
    c.crack = lhs.crack | rhs.crack;
    c.depression = lhs.depression | rhs.depression;
    c.displacement = lhs.displacement | rhs.displacement;
    c.vegetation = lhs.vegetation | rhs.vegetation;
    c.uneven = lhs.uneven | rhs.uneven;
    c.pothole = lhs.pothole | rhs.pothole;
    return c;
}

std::vector<BBox> Annot::CsvToBBox(const std::string &csv_path) {
    std::ifstream csvFile(csv_path);
    nlohmann::json json = csv_to_json(csvFile);
    std::vector<BBox> bboxes = group_box(json_to_box(json));
    return bboxes;
}

void Annot::PrintBBox(const std::vector<BBox> &boxes) {
    for (auto &bb : boxes) {
        std::cout << bb.image << std::endl;
        for (auto &bx : bb.boxes) {
            std::cout << "  " << bx.x << ", " << bx.y << ", " << bx.w << ", "
                      << bx.h << std::endl;
            std::cout << "    faults: " << bx.fault.ToStr() << std::endl;
        }
    }
}

// thickness of the bounding box
static constexpr int kThick = 5;
static constexpr int kFontFace = cv::FONT_HERSHEY_SIMPLEX;
static constexpr double kFontScale = 2.5;
static const cv::Scalar kTxtColor(255, 255, 255); // White
static const cv::Scalar kLineColor(0, 255, 0);    // Green

void Annot::ImgWrite(const BBox &bbx, const std::string &src,
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

void Annot::VisBBox(const std::string &dir_csv, const std::string &src,
                    const std::string &dst) {
    for (const auto &f : PathFinder::AllCsvs(dir_csv)) {
        std::vector<BBox> bboxes = Annot::CsvToBBox(f);
        for (auto &bbx : bboxes) {
            Annot::ImgWrite(bbx, src, dst);
        }
    }
}

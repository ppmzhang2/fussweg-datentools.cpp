#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "ibox.hpp"
#include "utils.hpp"

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
        {"bump",    "crack",  "depression", "displacement",
         "pothole", "uneven", "vegetation"};

    // Bounding Box-related constants
    static constexpr int kThickBorder = 15;
    static constexpr int kThickTxt = 8;
    static constexpr int kFontFace = cv::FONT_HERSHEY_SIMPLEX;
    static constexpr double kFontScale = 2.5;
    static const cv::Scalar kColorWhite(255, 255, 255);
    static const cv::Scalar kColorGreen(0, 255, 0);
    static const cv::Scalar kColorOrange(0, 165, 255);
    static const cv::Scalar kColorRed(0, 0, 255);

    static const std::array<const cv::Scalar, 4> kArrColor = {
        kColorWhite, kColorGreen, kColorOrange, kColorRed};

} // namespace

// Get distress level of a specific fault category
static inline constexpr uint8_t fault_level(const ibox::Fault &f,
                                            const uint8_t idx_cate) {
    return (static_cast<uint16_t>(f) >> (idx_cate * 2)) & 0b11;
}

// Type-wise MAX operator for Fault enum class to get the maximum level of
// distress
static inline void max_fault(ibox::Fault &lhs, ibox::Fault rhs) {
    uint8_t cond;
    for (uint8_t i = 0; i < kNFaultType; i++) {
        cond = MAX2(fault_level(lhs, i), fault_level(rhs, i));
        lhs = static_cast<ibox::Fault>(
            (static_cast<uint16_t>(lhs) & ~(0b11 << (i * 2))) |
            (cond << (i * 2)));
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
// One fault type and its severity level are connected by "-", while different
// fault types are connected by "_"
static inline std::string fault2str(ibox::Fault fault, const char c = '-',
                                    const char sep = '_') {
    uint8_t idx_level;
    std::string res = "";
    for (uint8_t idx_type = 0; idx_type < kNFaultType; idx_type++) {
        idx_level = fault_level(fault, idx_type);
        if (idx_level != 0) {
            if (!res.empty())
                res += sep;
            res += faulttype2str(idx_type) + c + faultlevel2str(idx_level);
        }
    }
    return res;
}

uint8_t ibox::Box::MaxSeverity() const {
    uint8_t max_lvl = 0; // Default severity (0 = None)

    // Iterate over the 7 fault categories
    for (uint8_t idx_cate = 0; idx_cate < kNFaultType; idx_cate++) {
        // Get the maximum severity level for each category
        max_lvl = MAX2(max_lvl, fault_level(this->fault, idx_cate));
    }

    return max_lvl;
}

// Convert Box to TSV strings. Format:
//
//  prefix,image,cate,level,x,y,w,h
std::string ibox::Box::ToTsv(const std::string &prefix) const {
    std::string res = "";
    uint8_t idx_level;
    for (uint8_t idx_type = 0; idx_type < kNFaultType; idx_type++) {
        idx_level = fault_level(this->fault, idx_type);
        if (idx_level != 0) {
            res += prefix + "\t" + image + "\t" + faulttype2str(idx_type) +
                   "\t" + faultlevel2str(idx_level) + "\t" + std::to_string(x) +
                   "\t" + std::to_string(y) + "\t" + std::to_string(w) + "\t" +
                   std::to_string(h) + "\n";
        }
    }
    return res;
}

std::string ibox::ImgBox::ToTsv(const std::string &prefix) const {
    std::string res = "";
    for (const auto &box : boxes) {
        res += box.ToTsv(prefix);
    }
    return res;
}

void ibox::ImgBox::Draw(const std::string &src, const std::string &dst) const {

    Fault fault_img = Fault::NONE;
    std::filesystem::path dir_src(src);
    std::filesystem::path dir_dst(dst);

    // Step 1: Read the Image
    cv::Mat img = cv::imread(dir_src / image, cv::IMREAD_COLOR);
    if (img.empty()) { // Check if the image is loaded
        std::cerr << "Error: Image " << image << " cannot be loaded!"
                  << std::endl;
        return;
    }

    // Step 2: Draw the Bounding Boxes
    for (const auto &bbx : this->boxes) {
        // get all faults for categorizing
        max_fault(fault_img, bbx.fault);

        // Step 2.1: draw the bounding box
        const cv::Rect box(bbx.x, bbx.y, bbx.w, bbx.h);
        cv::rectangle(img, box, kArrColor.at(bbx.MaxSeverity()), kThickBorder);

        // Step 2.2: fault text and size
        const char sep = '|';
        std::string txt = fault2str(bbx.fault, ' ', sep);
        std::vector<std::string> seq_txt;
        std::string _txt;
        std::istringstream iss(txt);
        while (std::getline(iss, _txt, sep)) {
            seq_txt.push_back(_txt);
        }
        int baseline = 0;
        cv::Size size_txt =
            cv::getTextSize(txt, kFontFace, kFontScale, kThickTxt, &baseline);
        baseline += kThickTxt;

        // // Step 2.3: Set background
        // const auto x_bg = bbx.x;
        // const auto y_bg = MAX2(1, bbx.y - size_txt.height);
        // const auto w_bg = MIN2(900, bbx.w); // TODO: magic 900
        // const auto h_bg =
        //     MIN2((size_txt.height + baseline) * int(seq_txt.size()), bbx.h);
        // cv::Rect rect_bg(x_bg, y_bg, w_bg, h_bg);
        // // Extract the ROI where the background rectangle will be drawn
        // cv::Mat roi = img(rect_bg);
        // // Fill ROI with the color_bg (black)
        // cv::Scalar color_bg(0, 0, 0);
        // cv::Mat bg_rect(roi.size(), roi.type(), color_bg);
        // bg_rect.setTo(color_bg);
        // // Blend the background rectangle with the ROI
        // cv::addWeighted(bg_rect, 0.5, roi, 0.5, 0.0, roi);
        // // Draw the background rectangle
        // cv::rectangle(img, rect_bg, color_bg, cv::FILLED);

        // Step 2.4: Write text line by line (OpenCV does not support newline)
        for (size_t i = 0; i < seq_txt.size(); ++i) {
            cv::putText(
                img, seq_txt[i],
                cv::Point(bbx.x, bbx.y + i * (size_txt.height + baseline)),
                kFontFace, kFontScale, kColorWhite, kThickTxt);
        }
    }

    // Step 3. Save the Image
    if (!cv::imwrite(dir_dst / (fault2str(fault_img, '_', '_') + "_" + image),
                     img)) {
        std::cerr << "Failed to save image" << std::endl;
    }
}

void ibox::drawBBox(const std::vector<ibox::ImgBox> &ibx_arr,
                    const std::string &src, const std::string &dst) {
    for (auto &ibox : ibx_arr) {
        ibox.Draw(src, dst);
    }
}

void ibox::toTsv(const std::vector<ibox::ImgBox> &ibx_arr,
                 const std::string &prefix, std::ostream &stream_o) {
    // Write the header
    stream_o << kTsvHeader << std::endl;
    // Loop through all CSV files
    for (const auto &ibx : ibx_arr) {
        stream_o << ibx.ToTsv(prefix);
    }
}

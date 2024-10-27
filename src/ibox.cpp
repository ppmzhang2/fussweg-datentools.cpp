#include <fstream>
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
        {"bump",       "crack",  "depression", "displacement",
         "vegetation", "uneven", "pothole"};
    inline static constexpr uint8_t kNFault = 21;

    // Bounding Box-related constants
    static constexpr int kThick = 5;
    static constexpr int kFontFace = cv::FONT_HERSHEY_SIMPLEX;
    static constexpr double kFontScale = 2.5;
    static const cv::Scalar kTxtColor(255, 255, 255); // White
    static const cv::Scalar kLineColor(0, 255, 0);    // Green

} // namespace

// Type-wise MAX operator for Fault enum class to get the maximum level of
// distress
static inline void max_fault(ibox::Fault &lhs, ibox::Fault rhs) {
    for (int i = 0; i < kNFaultType; i++) {
        uint8_t cond_lhs = (static_cast<uint16_t>(lhs) >> (i * 2)) & 0b11;
        uint8_t cond_rhs = (static_cast<uint16_t>(rhs) >> (i * 2)) & 0b11;
        uint8_t cond = MAX2(cond_lhs, cond_rhs);
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
std::string fault2str(ibox::Fault fault) {
    std::string res = "";
    for (int ind_type = 0; ind_type < kNFaultType; ind_type++) {
        uint8_t ind_level =
            (static_cast<uint16_t>(fault) >> (ind_type * 2)) & 0b11;
        if (ind_level != 0) {
            if (!res.empty())
                res += "_";
            res += faulttype2str(ind_type) + "-" + faultlevel2str(ind_level);
        }
    }
    return res;
}

static inline std::array<uint8_t, kNFault> fault2count(ibox::Fault fault) {
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
ibox::ImgFaultCount::ImgFaultCount()
    : image(""), bump_fair(0), bump_poor(0), bump_verypoor(0), crack_fair(0),
      crack_poor(0), crack_verypoor(0), depression_fair(0), depression_poor(0),
      depression_verypoor(0), displacement_fair(0), displacement_poor(0),
      displacement_verypoor(0), vegetation_fair(0), vegetation_poor(0),
      vegetation_verypoor(0), uneven_fair(0), uneven_poor(0),
      uneven_verypoor(0), pothole_fair(0), pothole_poor(0),
      pothole_verypoor(0) {}

// constructor with ImgBox
ibox::ImgFaultCount::ImgFaultCount(const ImgBox &ibox)
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

std::string ibox::ImgFaultCount::ToStr() const {
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

void ibox::FaultStats::AddFault(const Fault &f) {
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

// Convert Box to TSV strings. Format:
//
//  prefix,image,cate,level,x,y,w,h
std::string ibox::Box::ToTsv(const std::string &group) const {
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

std::string ibox::ImgBox::ToTsv(const std::string &group) const {
    std::string res = "";
    for (const auto &box : boxes) {
        res += box.ToTsv(group);
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
    for (const auto &bbx : boxes) {
        // get all faults for categorizing
        max_fault(fault_img, bbx.fault);

        cv::Rect box(bbx.x, bbx.y, bbx.w, bbx.h);
        // Positioning the text above the box
        cv::Point pt_txt_org(box.x, box.y);

        // Draw bounding box
        cv::rectangle(img, box, kLineColor, kThick);

        // Add text background
        std::string txt = fault2str(bbx.fault);

        // Step 2.1: Measure text size
        int baseline = 0;
        cv::Size size_txt =
            cv::getTextSize(txt, kFontFace, kFontScale, kThick, &baseline);
        baseline += kThick; // Adjust for thickness

        // Step 3: Draw text on top of the background
        std::vector<std::string> seq_txt;
        std::string _txt;
        std::istringstream iss(txt);
        while (std::getline(iss, _txt, '_')) {
            seq_txt.push_back(_txt);
        }
        // 900 is the width of the longest text
        cv::Rect rect_bg(pt_txt_org.x, pt_txt_org.y - size_txt.height, 900,
                         (size_txt.height + baseline) * seq_txt.size());
        // Extract the region of interest (ROI) where the background rectangle
        // will be drawn
        cv::Mat roi = img(rect_bg);
        // Create a background image with the same size as the ROI and fill it
        // with the color_bg (green)
        cv::Scalar color_bg(0, 0, 0);
        cv::Mat bg_rect(roi.size(), roi.type(), color_bg);
        bg_rect.setTo(color_bg);
        // Blend the background rectangle with the ROI
        cv::addWeighted(bg_rect, 0.5, roi, 0.5, 0.0, roi);

        cv::rectangle(img, rect_bg, color_bg, cv::FILLED);

        for (size_t i = 0; i < seq_txt.size(); ++i) {
            cv::putText(
                img, seq_txt[i],
                cv::Point(pt_txt_org.x,
                          pt_txt_org.y + i * (size_txt.height + baseline)),
                kFontFace, kFontScale, kTxtColor, kThick);
        }
    }

    // Step 3. Save the Image
    if (!cv::imwrite(dir_dst / (fault2str(fault_img) + "_" + image), img)) {
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
                 const std::string &group, std::ostream &stream_o) {
    // Write the header
    stream_o << kTsvHeader << std::endl;
    // Loop through all CSV files
    for (const auto &ibx : ibx_arr) {
        stream_o << ibx.ToTsv(group);
    }
}

void ibox::exportStats(const std::vector<ibox::ImgBox> &ibx_arr,
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

    for (const auto &ibx : ibx_arr) {
        ImgFaultCount fc = ImgFaultCount(ibx);
        file << fc.ToStr() << "\n";
    }
}

void ibox::printStats(const std::vector<ibox::ImgBox> &ibx_arr) {
    FaultStats stats;
    unsigned int img_cnt = 0;

    for (auto &ibx : ibx_arr) {
        for (auto &bx : ibx.boxes) {
            stats.AddFault(bx.fault);
        }
    }
    img_cnt += ibx_arr.size();
    std::cout << "Total images: " << img_cnt << std::endl;
    stats.Print();
}

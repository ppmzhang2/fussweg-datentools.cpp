#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

#include "annot.hpp"
#include "config.h"
#include "exif_json.hpp"
#include "optical_flow.hpp"
#include "pov.hpp"

void writeToFile(const std::string &out_file, const std::string &content) {
    std::ofstream outFile(out_file);
    if (!outFile) {
        throw std::runtime_error("Unable to open for writing: " + out_file);
    }
    outFile << content << std::endl;
    outFile.close();
}

int parse_args(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << "Fussweg Datentools" << std::endl;
        std::cout << "Version: " << VERSION_MAJOR << "." << VERSION_MINOR << "."
                  << VERSION_PATCH << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: " << std::endl;
        std::cout << "  " << argv[0] << " exif <directory_path> "
                  << "<output_file_path>" << std::endl;
        std::cout << "  " << argv[0] << " displacement <directory_path> "
                  << "<output_file_path>" << std::endl;
        std::cout << "  " << argv[0] << " pov-roi <width> <height> \\\n"
                  << "    <vp_t_bl_x> <vp_t_bl_y> <vp_t_tl_x> <vp_t_tl_y> \\\n"
                  << "    <vp_t_br_x> <vp_t_br_y> <vp_t_tr_x> <vp_t_tr_y> \\\n"
                  << "    <vp_s_tl_x> <vp_s_tl_y> <vp_s_tr_x> <vp_s_tr_y> \\\n"
                  << "    <vp_s_bl_x> <vp_s_bl_y> <vp_s_br_x> <vp_s_br_y>"
                  << std::endl;
        std::cout << "  " << argv[0] << " pov-transform <width> <height> \\\n"
                  << "    <roi_tl_x> <roi_tl_y> <roi_tr_x> <roi_tr_y> \\\n"
                  << "    <roi_br_x> <roi_br_y> <roi_bl_x> <roi_bl_y> \\\n"
                  << "    <dir_src> <dir_dst>" << std::endl;
        return 1;
    }

    std::string op = argv[1];
    if (op != "exif" && op != "displacement" && op != "bbox-csv" &&
        op != "bbox-stat" && op != "pov-roi" && op != "pov-transform") {
        throw std::runtime_error("Unknown operation. "
                                 "Use 'exif', 'displacement', "
                                 "'pov-roi' or 'pov-transform'.");
    }
    if (op == "exif" && argc != 4) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "displacement" && argc != 4) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "bbox-csv" && argc != 5) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "bbox-stat" && argc != 3) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "pov-roi" && argc != 20) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "pov-transform" && argc != 14) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "exif") {
        std::string dir_path = argv[2];
        std::string out_path = argv[3];
        auto out = ExifJson::FromFiles(dir_path);
        writeToFile(out_path, out.dump(4));
        return 0;
    }
    if (op == "displacement") {
        std::string dir_path = argv[2];
        std::string out_path = argv[3];
        auto out = OpticalFlow::Displacement(dir_path);
        writeToFile(out_path, out.dump(4));
        return 0;
    }
    if (op == "bbox-csv") {
        std::string dir_csv = argv[2];
        std::string dir_src = argv[3];
        std::string dir_dst = argv[4];
        Annot::VisBBox(dir_csv, dir_src, dir_dst);
        return 0;
    }
    if (op == "bbox-stat") {
        std::string dir_csv = argv[2];
        Annot::Stats(dir_csv);
        return 0;
    }
    if (op == "pov-roi") {
        int width = std::strtol(argv[2], nullptr, 10);
        int height = std::strtol(argv[3], nullptr, 10);
        // coordinates of calculate top vanishing point
        double vp_t_bl_x = std::strtol(argv[4], nullptr, 10);
        double vp_t_bl_y = std::strtol(argv[5], nullptr, 10);
        double vp_t_tl_x = std::strtol(argv[6], nullptr, 10);
        double vp_t_tl_y = std::strtol(argv[7], nullptr, 10);
        double vp_t_br_x = std::strtol(argv[8], nullptr, 10);
        double vp_t_br_y = std::strtol(argv[9], nullptr, 10);
        double vp_t_tr_x = std::strtol(argv[10], nullptr, 10);
        double vp_t_tr_y = std::strtol(argv[11], nullptr, 10);
        // coordinates of calculate side vanishing point
        double vp_s_tl_x = std::strtol(argv[12], nullptr, 10);
        double vp_s_tl_y = std::strtol(argv[13], nullptr, 10);
        double vp_s_tr_x = std::strtol(argv[14], nullptr, 10);
        double vp_s_tr_y = std::strtol(argv[15], nullptr, 10);
        double vp_s_bl_x = std::strtol(argv[16], nullptr, 10);
        double vp_s_bl_y = std::strtol(argv[17], nullptr, 10);
        double vp_s_br_x = std::strtol(argv[18], nullptr, 10);
        double vp_s_br_y = std::strtol(argv[19], nullptr, 10);

        Quad roi = {
            .tl = new cv::Point2d{0, 0},
            .tr = new cv::Point2d{0, 0},
            .br = new cv::Point2d{0, 0},
            .bl = new cv::Point2d{0, 0},
        };

        Quad vert = {
            .tl = new cv::Point2d{vp_t_tl_x, vp_t_tl_y},
            .tr = new cv::Point2d{vp_t_tr_x, vp_t_tr_y},
            .br = new cv::Point2d{vp_t_br_x, vp_t_br_y},
            .bl = new cv::Point2d{vp_t_bl_x, vp_t_bl_y},
        };
        Quad horz = {
            .tl = new cv::Point2d{vp_s_tl_x, vp_s_tl_y},
            .tr = new cv::Point2d{vp_s_tr_x, vp_s_tr_y},
            .br = new cv::Point2d{vp_s_br_x, vp_s_br_y},
            .bl = new cv::Point2d{vp_s_bl_x, vp_s_bl_y},
        };
        PoV::get_roi(height, width, vert, horz, roi);
        roi.print();
        return 0;
    }
    if (op == "pov-transform") {
        const int width = std::strtol(argv[2], nullptr, 10);
        const int height = std::strtol(argv[3], nullptr, 10);
        // coordinates of ROI
        const double roi_tl_x = std::strtol(argv[4], nullptr, 10);
        const double roi_tl_y = std::strtol(argv[5], nullptr, 10);
        const double roi_tr_x = std::strtol(argv[6], nullptr, 10);
        const double roi_tr_y = std::strtol(argv[7], nullptr, 10);
        const double roi_br_x = std::strtol(argv[8], nullptr, 10);
        const double roi_br_y = std::strtol(argv[9], nullptr, 10);
        const double roi_bl_x = std::strtol(argv[10], nullptr, 10);
        const double roi_bl_y = std::strtol(argv[11], nullptr, 10);
        const std::string dir_src = argv[12];
        const std::string dir_dst = argv[13];

        const Quad roi = {
            .tl = new cv::Point2d{roi_tl_x, roi_tl_y},
            .tr = new cv::Point2d{roi_tr_x, roi_tr_y},
            .br = new cv::Point2d{roi_br_x, roi_br_y},
            .bl = new cv::Point2d{roi_bl_x, roi_bl_y},
        };
        PoV::transform(roi, width, height, dir_src, dir_dst);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    try {
        return parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

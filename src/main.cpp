#include <iostream>

#include "annot.hpp"
#include "config.h"
#include "crs.hpp"
#include "cv.hpp"
#include "exif.hpp"
#include "gis.hpp"
#include "utils.hpp"

int parse_args(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << "Fussweg Datentools" << std::endl;
        std::cout << "Version: " << VERSION_MAJOR << "." << VERSION_MINOR << "."
                  << VERSION_PATCH << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: " << std::endl;
        std::cout << "  " << argv[0] << " exif-export-json "
                  << "<directory_path> <output_file_path>" << std::endl;
        std::cout << "  " << argv[0] << " exif-export-csv "
                  << "<directory_path> <output_file_path>" << std::endl;
        std::cout << "  " << argv[0] << " displacement "
                  << "<directory_path> <output_file_path>" << std::endl;
        std::cout << "  " << argv[0] << " annot-export "
                  << "<label_dir> <group> <out_file_path>" << std::endl;
        std::cout << "  " << argv[0] << " annot-export-stats "
                  << "<label_dir> <out_file_path>" << std::endl;
        std::cout << "  " << argv[0] << " annot-print-stats "
                  << "<label_dir>" << std::endl;
        std::cout << "  " << argv[0] << " bbox-draw "
                  << "<label_dir> <src_dir> <dst_dir> <ext>" << std::endl;
        std::cout << "  " << argv[0] << " crs-to-nzgd2000 "
                  << "<latitude> <longitude>" << std::endl;
        std::cout << "  " << argv[0] << " crs-from-nzgd2000 "
                  << "<easting> <northing>" << std::endl;
        std::cout << "  " << argv[0] << " geojson-to-tsv "
                  << "<geojson_file> <output_file>" << std::endl;
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
    if (op != "exif-export-json" && op != "exif-export-csv" &&
        op != "displacement" && op != "annot-export" &&
        op != "annot-print-stats" && op != "annot-export-stats" &&
        op != "bbox-draw" && op != "pov-roi" && op != "pov-transform" &&
        op != "crs-to-nzgd2000" && op != "crs-from-nzgd2000" &&
        op != "geojson-to-tsv") {
        throw std::runtime_error("Unknown operation. ");
    }
    if ((op == "exif-export-json" && argc != 4) ||
        (op == "exif-export-csv" && argc != 4) ||
        (op == "displacement" && argc != 4) ||
        (op == "annot-export" && argc != 5) ||
        (op == "annot-export-stats" && argc != 4) ||
        (op == "annot-print-stats" && argc != 3) ||
        (op == "bbox-draw" && argc != 6) ||
        (op == "geojson-to-tsv" && argc != 4) ||
        (op == "crs-to-nzgd2000" && argc != 4) ||
        (op == "crs-from-nzgd2000" && argc != 4) ||
        (op == "pov-roi" && argc != 20) ||
        (op == "pov-transform" && argc != 14)) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "exif-export-json") {
        std::string dir_path = argv[2];
        std::string out_path = argv[3];
        std::ofstream out_file(out_path);
        fdt::exif::exportJson(dir_path, out_file);
        out_file.close();
        return 0;
    }
    if (op == "exif-export-csv") {
        std::string dir_path = argv[2];
        std::string out_path = argv[3];
        std::ofstream out_file(out_path);
        fdt::exif::exportCsv(dir_path, out_file);
        out_file.close();
        return 0;
    }
    if (op == "displacement") {
        std::string dir_path = argv[2];
        std::string out_path = argv[3];
        auto out = fdt::cv::getOfDiffJson(dir_path);
        fdt::utils::writeFile(out_path, out.dump(4));
        return 0;
    }
    if (op == "bbox-draw") {
        std::string dir_lab = argv[2];
        std::string dir_src = argv[3];
        std::string dir_dst = argv[4];
        std::string ext = argv[5];
        // TODO: make it more robust
        if (ext == "json") {
            ext = ".json";
        } else {
            ext = ".csv";
        }
        fdt::annot::drawImgBoxes(dir_lab, dir_src, dir_dst, ext);
        return 0;
    }
    if (op == "annot-export") {
        std::string dir_lab = argv[2];
        std::string group = argv[3];
        std::string tsv_file = argv[4];
        std::ofstream stream_of(tsv_file);
        fdt::annot::exportTsv(dir_lab, group, stream_of);
        stream_of.close();
        return 0;
    }
    if (op == "annot-export-stats") {
        std::string dir_lab = argv[2];
        std::string csv_file = argv[3];
        fdt::annot::exportStats(dir_lab, csv_file);
        return 0;
    }
    if (op == "annot-print-stats") {
        std::string dir_lab = argv[2];
        fdt::annot::printStats(dir_lab);
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

        fdt::cv::Quad roi = {
            .tl = new cv::Point2d{0, 0},
            .tr = new cv::Point2d{0, 0},
            .br = new cv::Point2d{0, 0},
            .bl = new cv::Point2d{0, 0},
        };

        fdt::cv::Quad vert = {
            .tl = new cv::Point2d{vp_t_tl_x, vp_t_tl_y},
            .tr = new cv::Point2d{vp_t_tr_x, vp_t_tr_y},
            .br = new cv::Point2d{vp_t_br_x, vp_t_br_y},
            .bl = new cv::Point2d{vp_t_bl_x, vp_t_bl_y},
        };
        fdt::cv::Quad horz = {
            .tl = new cv::Point2d{vp_s_tl_x, vp_s_tl_y},
            .tr = new cv::Point2d{vp_s_tr_x, vp_s_tr_y},
            .br = new cv::Point2d{vp_s_br_x, vp_s_br_y},
            .bl = new cv::Point2d{vp_s_bl_x, vp_s_bl_y},
        };
        fdt::cv::getQuadRoi(height, width, vert, horz, roi);
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

        const fdt::cv::Quad roi = {
            .tl = new cv::Point2d{roi_tl_x, roi_tl_y},
            .tr = new cv::Point2d{roi_tr_x, roi_tr_y},
            .br = new cv::Point2d{roi_br_x, roi_br_y},
            .bl = new cv::Point2d{roi_bl_x, roi_bl_y},
        };
        fdt::cv::transPerspe(roi, width, height, dir_src, dir_dst);
        return 0;
    }
    if (op == "crs-to-nzgd2000") {
        double lat = std::strtod(argv[2], nullptr);
        double lon = std::strtod(argv[3], nullptr);
        auto [e, n] = fdt::crs::ToNzgd2000(lat, lon);
        std::cout << "Easting: " << e << ", Northing: " << n << std::endl;
        return 0;
    }
    if (op == "crs-from-nzgd2000") {
        double e = std::strtod(argv[2], nullptr);
        double n = std::strtod(argv[3], nullptr);
        auto [lat, lon] = fdt::crs::FromNzgd2000(e, n);
        std::cout << "Latitude: " << lat << ", Longitude: " << lon << std::endl;
        return 0;
    }
    if (op == "geojson-to-tsv") {
        std::ifstream geojson_file(argv[2]);
        std::ofstream output_file(argv[3]);
        fdt::gis::Geojson2Tsv(geojson_file, output_file);
        geojson_file.close();
        output_file.close();
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

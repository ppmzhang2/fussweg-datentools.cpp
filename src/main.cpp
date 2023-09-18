#include "config.h"
#include "exif_json.hpp"
#include "optical_flow.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

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
        std::cout << "  " << argv[0] << " displacement <directory_path>"
                  << "<output_file_path>" << std::endl;
        return 1;
    }

    std::string op = argv[1];
    if (op != "exif" && op != "displacement") {
        throw std::runtime_error("Unknown operation. "
                                 "Use 'exif' or 'displacement'.");
    }
    if (op == "exif" && argc != 4) {
        throw std::runtime_error("Invalid number of arguments.");
    }
    if (op == "displacement" && argc != 4) {
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

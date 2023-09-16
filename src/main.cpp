#include "config.h"
#include "exif.hpp"
#include "exif_serde.hpp"
#include "file_iter.hpp"
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

    for (const auto &path : FileIter(content)) {
        std::string j = ExifSerde::Serialize(Exif::GetAttrs(path));
        outFile << j << std::endl;
    }
    outFile.close();
}

int main(int argc, char *argv[]) {
    // if (argc != 3) {
    //     std::cout << "Fussweg Datentools" << std::endl;
    //     std::cout << "Version: " << VERSION_MAJOR << "." << VERSION_MINOR <<
    //     "."
    //               << VERSION_PATCH << std::endl;
    //     std::cout << std::endl;
    //     std::cerr << "Usage: " << argv[0]
    //               << " <directory_path> <output_file_path>" << std::endl;
    //     return 1;
    // }

    std::string dir_path = argv[1];

    nlohmann::json j = OpticalFlow::Displacement(dir_path);
    std::cout << j << std::endl;
    // std::string out_path = argv[2];
    // writeToFile(out_path, dir_path);
}

#include "config.h"
#include "exif.hpp"
#include <filesystem>
#include <iomanip>
#include <iostream>

int main() {
    std::cout << "Fussweg Datentools" << std::endl;
    std::cout << "Version: " << VERSION_MAJOR << "." << VERSION_MINOR << "."
              << VERSION_PATCH << std::endl;
    // get Geo-Location of a picture
    const std::string kRelPth = "./data/DSCN0010.jpg";
    const std::string kAbsPth = std::filesystem::absolute(kRelPth);
    // Coord coor = GetCoord(kAbsPth);
    // std::cout << "Lat:" << std::fixed << std::setprecision(15) << coor.lat
    //           << std::endl;
    // std::cout << "Lon:" << std::fixed << std::setprecision(15) << coor.lon
    //           << std::endl;

    ExifAttrs attributes = GetExifAttrs(kAbsPth);
    // Print the attributes
    PrintAttrs(attributes);
    // ListAllAttrs(kAbsPth);
    std::cin.get();
}

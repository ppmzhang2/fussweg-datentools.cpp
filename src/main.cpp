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
    const std::string relPth = "./data/DSCN0010.jpg";
    const std::string absPth = std::filesystem::absolute(relPth);
    Coord latLon = getCoord(absPth);
    std::cout << "Lat:" << std::fixed << std::setprecision(15) << latLon.lat
              << std::endl;
    std::cout << "Lon:" << std::fixed << std::setprecision(15) << latLon.lon
              << std::endl;
    std::cin.get();
}

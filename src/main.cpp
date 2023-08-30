#include "config.h"
#include <iostream>
#include <ostream>

int main() {
    std::cout << "Fussweg Datentools" << std::endl;
    std::cout << "Version: " << VERSION_MAJOR << "." << VERSION_MINOR << "."
              << VERSION_PATCH << std::endl;
    std::cin.get();
}

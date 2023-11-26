#include "path_finder.hpp"

Paths PathFinder::AllFiles(const std::string &dir) {
    Paths imgs;
    for (const auto &f : std::filesystem::recursive_directory_iterator(dir)) {
        if (f.path().extension() == ".JPG" || f.path().extension() == ".jpg") {
            imgs.push_back(f.path().string());
        }
    }
    return imgs;
}

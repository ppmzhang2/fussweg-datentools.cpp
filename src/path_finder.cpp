#include "path_finder.hpp"

Paths PathFinder::AllImages(const std::string &dir) {
    Paths imgs;
    for (const auto &f : std::filesystem::recursive_directory_iterator(dir)) {
        if (f.path().extension() == ".JPG" || f.path().extension() == ".jpg") {
            imgs.push_back(f.path().string());
        }
    }
    return imgs;
}

Paths PathFinder::AllFiles(const std::string &dir, const std::string &ext) {
    Paths files;
    std::string ext_l = ext;
    std::string ext_u = ext;
    std::transform(ext_l.begin(), ext_l.end(), ext_l.begin(), ::tolower);
    std::transform(ext_u.begin(), ext_u.end(), ext_u.begin(), ::toupper);
    for (const auto &f : std::filesystem::recursive_directory_iterator(dir)) {
        if (f.path().extension() == ext_l || f.path().extension() == ext_u) {
            files.push_back(f.path().string());
        }
    }
    return files;
}

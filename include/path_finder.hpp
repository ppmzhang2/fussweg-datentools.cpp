#pragma once

#include <filesystem>
#include <vector>

typedef std::vector<std::string> Paths;

namespace PathFinder {

    Paths AllImages(const std::string &);

    Paths AllCsvs(const std::string &);

} // namespace PathFinder

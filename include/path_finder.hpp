#pragma once

#include <algorithm>
#include <filesystem>
#include <vector>

typedef std::vector<std::string> Paths;

namespace PathFinder {

    Paths AllImages(const std::string &);

    Paths AllFiles(const std::string &, const std::string &);

} // namespace PathFinder

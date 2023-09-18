#pragma once

#include <filesystem>
#include <vector>

typedef std::vector<std::string> Paths;

namespace PathFinder {
    Paths AllFiles(const std::string &);
} // namespace PathFinder

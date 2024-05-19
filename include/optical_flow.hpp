#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace OpticalFlow {
    nlohmann::json Displacement(const std::string &);
} // namespace OpticalFlow

#pragma once

#include <iostream>

struct Box {
    int x;
    int y;
    int w;
    int h;
    std::string image;
    std::array<bool, 7> faults;
    std::array<bool, 3> conds;
};

struct BBox {
    std::string image;
    std::vector<Box> boxes;
};

namespace Annot {

    static const std::array<std::string, 7> kFaultNames = {
        "bump",       "crack",  "depression", "displacement",
        "vegetation", "uneven", "pothole"};

    static const std::array<std::string, 3> kCondNames = {
        "fair",
        "poor",
        "verypoor",
    };

    // map Fault to string
    std::vector<std::string> FaultToStr(const Box &);

    // map Condition to string
    std::vector<std::string> CondToStr(const Box &);

    void PrintBBox(const std::vector<BBox> &);

    std::vector<BBox> CsvToBBox(const std::string &);

    void ImgWrite(const BBox &, const std::string &, const std::string &);

    void VisBBox(const std::string &, const std::string &, const std::string &);

} // namespace Annot

#pragma once

#include <iostream>

struct Fault {
    unsigned int bump : 2;
    unsigned int crack : 2;
    unsigned int depression : 2;
    unsigned int displacement : 2;
    unsigned int vegetation : 2;
    unsigned int uneven : 2;
    unsigned int pothole : 2;
    unsigned int padding : 2;

    // Constructor
    Fault()
        : bump(0), crack(0), depression(0), displacement(0), vegetation(0),
          uneven(0), pothole(0), padding(0) {}

    std::string ToStr() const {
        const std::string faults[7] = {
            fault_to_str(bump, "bump"),
            fault_to_str(crack, "crack"),
            fault_to_str(depression, "depression"),
            fault_to_str(displacement, "displacement"),
            fault_to_str(vegetation, "vegetation"),
            fault_to_str(uneven, "uneven"),
            fault_to_str(pothole, "pothole"),

        };
        std::string res = "";
        for (const auto &f : faults) {
            if (!f.empty() && res.empty()) {
                res = f;
            } else if (!f.empty()) {
                res += "_" + f;
            } else {
                continue;
            }
        }
        return res;
    }

  private:
    std::string fault_to_str(unsigned int cond,
                             const std::string &fault) const {
        switch (cond) {
        case 1:
            return fault + "_fair";
        case 2:
            return fault + "_poor";
        case 3:
            return fault + "_verypoor";
        default:
            return "";
        }
    }
};

struct Box {
    int x;
    int y;
    int w;
    int h;
    std::string image;
    Fault fault;
};

struct BBox {
    std::string image;
    std::vector<Box> boxes;
};

namespace Annot {

    void PrintBBox(const std::vector<BBox> &);

    std::vector<BBox> CsvToBBox(const std::string &);

    void ImgWrite(const BBox &, const std::string &, const std::string &);

    void VisBBox(const std::string &, const std::string &, const std::string &);

} // namespace Annot

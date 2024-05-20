#pragma once

#include <iostream>
#include <vector>
#define MAX2(x, y) ((x) > (y) ? (x) : (y))

namespace fdt {
    namespace annot {
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
                : bump(0), crack(0), depression(0), displacement(0),
                  vegetation(0), uneven(0), pothole(0), padding(0) {}

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

        using BoxArr = std::vector<Box>;

        struct ImgBox {
            std::string image;
            BoxArr boxes;

            void Print() const {
                std::cout << image << std::endl;
                for (auto &bx : boxes) {
                    std::cout << "  " << bx.x << ", " << bx.y << ", " << bx.w
                              << ", " << bx.h << std::endl;
                    std::cout << "    faults: " << bx.fault.ToStr()
                              << std::endl;
                }
            }
        };

        using ImgBoxArr = std::vector<ImgBox>;

        struct FaultCount {
            std::string image;
            uint8_t bump_fair;
            uint8_t bump_poor;
            uint8_t bump_verypoor;
            uint8_t crack_fair;
            uint8_t crack_poor;
            uint8_t crack_verypoor;
            uint8_t depression_fair;
            uint8_t depression_poor;
            uint8_t depression_verypoor;
            uint8_t displacement_fair;
            uint8_t displacement_poor;
            uint8_t displacement_verypoor;
            uint8_t vegetation_fair;
            uint8_t vegetation_poor;
            uint8_t vegetation_verypoor;
            uint8_t uneven_fair;
            uint8_t uneven_poor;
            uint8_t uneven_verypoor;
            uint8_t pothole_fair;
            uint8_t pothole_poor;
            uint8_t pothole_verypoor;

            // constructor
            FaultCount()
                : image(""), bump_fair(0), bump_poor(0), bump_verypoor(0),
                  crack_fair(0), crack_poor(0), crack_verypoor(0),
                  depression_fair(0), depression_poor(0),
                  depression_verypoor(0), displacement_fair(0),
                  displacement_poor(0), displacement_verypoor(0),
                  vegetation_fair(0), vegetation_poor(0),
                  vegetation_verypoor(0), uneven_fair(0), uneven_poor(0),
                  uneven_verypoor(0), pothole_fair(0), pothole_poor(0),
                  pothole_verypoor(0) {}

            void FromBBox(const ImgBox &bbox) {
                image = bbox.image;
                for (const auto &box : bbox.boxes) {
                    bump_fair += box.fault.bump == 1;
                    bump_poor += box.fault.bump == 2;
                    bump_verypoor += box.fault.bump == 3;
                    crack_fair += box.fault.crack == 1;
                    crack_poor += box.fault.crack == 2;
                    crack_verypoor += box.fault.crack == 3;
                    depression_fair += box.fault.depression == 1;
                    depression_poor += box.fault.depression == 2;
                    depression_verypoor += box.fault.depression == 3;
                    displacement_fair += box.fault.displacement == 1;
                    displacement_poor += box.fault.displacement == 2;
                    displacement_verypoor += box.fault.displacement == 3;
                    vegetation_fair += box.fault.vegetation == 1;
                    vegetation_poor += box.fault.vegetation == 2;
                    vegetation_verypoor += box.fault.vegetation == 3;
                    uneven_fair += box.fault.uneven == 1;
                    uneven_poor += box.fault.uneven == 2;
                    uneven_verypoor += box.fault.uneven == 3;
                    pothole_fair += box.fault.pothole == 1;
                    pothole_poor += box.fault.pothole == 2;
                    pothole_verypoor += box.fault.pothole == 3;
                }
            }

            std::string ToStr() const {
                return image + "," + std::to_string(bump_fair) + "," +
                       std::to_string(bump_poor) + "," +
                       std::to_string(bump_verypoor) + "," +
                       std::to_string(crack_fair) + "," +
                       std::to_string(crack_poor) + "," +
                       std::to_string(crack_verypoor) + "," +
                       std::to_string(depression_fair) + "," +
                       std::to_string(depression_poor) + "," +
                       std::to_string(depression_verypoor) + "," +
                       std::to_string(displacement_fair) + "," +
                       std::to_string(displacement_poor) + "," +
                       std::to_string(displacement_verypoor) + "," +
                       std::to_string(vegetation_fair) + "," +
                       std::to_string(vegetation_poor) + "," +
                       std::to_string(vegetation_verypoor) + "," +
                       std::to_string(uneven_fair) + "," +
                       std::to_string(uneven_poor) + "," +
                       std::to_string(uneven_verypoor) + "," +
                       std::to_string(pothole_fair) + "," +
                       std::to_string(pothole_poor) + "," +
                       std::to_string(pothole_verypoor);
            }
        };

        struct FaultStats {
            unsigned int bump_fair;
            unsigned int bump_poor;
            unsigned int bump_verypoor;
            unsigned int crack_fair;
            unsigned int crack_poor;
            unsigned int crack_verypoor;
            unsigned int depression_fair;
            unsigned int depression_poor;
            unsigned int depression_verypoor;
            unsigned int displacement_fair;
            unsigned int displacement_poor;
            unsigned int displacement_verypoor;
            unsigned int vegetation_fair;
            unsigned int vegetation_poor;
            unsigned int vegetation_verypoor;
            unsigned int uneven_fair;
            unsigned int uneven_poor;
            unsigned int uneven_verypoor;
            unsigned int pothole_fair;
            unsigned int pothole_poor;
            unsigned int pothole_verypoor;

            // constructor
            FaultStats()
                : bump_fair(0), bump_poor(0), bump_verypoor(0), crack_fair(0),
                  crack_poor(0), crack_verypoor(0), depression_fair(0),
                  depression_poor(0), depression_verypoor(0),
                  displacement_fair(0), displacement_poor(0),
                  displacement_verypoor(0), vegetation_fair(0),
                  vegetation_poor(0), vegetation_verypoor(0), uneven_fair(0),
                  uneven_poor(0), uneven_verypoor(0), pothole_fair(0),
                  pothole_poor(0), pothole_verypoor(0) {}

            // add fault
            void AddFault(const Fault &f) {
                bump_fair += f.bump == 1;
                bump_poor += f.bump == 2;
                bump_verypoor += f.bump == 3;
                crack_fair += f.crack == 1;
                crack_poor += f.crack == 2;
                crack_verypoor += f.crack == 3;
                depression_fair += f.depression == 1;
                depression_poor += f.depression == 2;
                depression_verypoor += f.depression == 3;
                displacement_fair += f.displacement == 1;
                displacement_poor += f.displacement == 2;
                displacement_verypoor += f.displacement == 3;
                vegetation_fair += f.vegetation == 1;
                vegetation_poor += f.vegetation == 2;
                vegetation_verypoor += f.vegetation == 3;
                uneven_fair += f.uneven == 1;
                uneven_poor += f.uneven == 2;
                uneven_verypoor += f.uneven == 3;
                pothole_fair += f.pothole == 1;
                pothole_poor += f.pothole == 2;
                pothole_verypoor += f.pothole == 3;
            }

            void Print() const {
                std::cout << "bump_fair: " << bump_fair << std::endl;
                std::cout << "bump_poor: " << bump_poor << std::endl;
                std::cout << "bump_verypoor: " << bump_verypoor << std::endl;
                std::cout << "crack_fair: " << crack_fair << std::endl;
                std::cout << "crack_poor: " << crack_poor << std::endl;
                std::cout << "crack_verypoor: " << crack_verypoor << std::endl;
                std::cout << "depression_fair: " << depression_fair
                          << std::endl;
                std::cout << "depression_poor: " << depression_poor
                          << std::endl;
                std::cout << "depression_verypoor: " << depression_verypoor
                          << std::endl;
                std::cout << "displacement_fair: " << displacement_fair
                          << std::endl;
                std::cout << "displacement_poor: " << displacement_poor
                          << std::endl;
                std::cout << "displacement_verypoor: " << displacement_verypoor
                          << std::endl;
                std::cout << "vegetation_fair: " << vegetation_fair
                          << std::endl;
                std::cout << "vegetation_poor: " << vegetation_poor
                          << std::endl;
                std::cout << "vegetation_verypoor: " << vegetation_verypoor
                          << std::endl;
                std::cout << "uneven_fair: " << uneven_fair << std::endl;
                std::cout << "uneven_poor: " << uneven_poor << std::endl;
                std::cout << "uneven_verypoor: " << uneven_verypoor
                          << std::endl;
                std::cout << "pothole_fair: " << pothole_fair << std::endl;
                std::cout << "pothole_poor: " << pothole_poor << std::endl;
                std::cout << "pothole_verypoor: " << pothole_verypoor
                          << std::endl;
            }
        };

        ImgBoxArr parseCsv(const std::string &);

        ImgBoxArr parseJson(const std::string &);

        void drawImgBox(const ImgBox &, const std::string &,
                        const std::string &);

        void drawImgBoxes(const std::string &, const std::string &,
                          const std::string &, const std::string &);

        void exportStats(const std::string &, const std::string &);

        void printStats(const std::string &);

    } // namespace annot
} // namespace fdt

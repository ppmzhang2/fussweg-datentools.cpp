#pragma once

#include <iostream>
#include <vector>

namespace fdt {
    namespace via {

        // Define the Fault enum class with bitmask values
        enum class Fault : uint16_t {
            NONE = 0,
            BUMP_FAIR = 0b01 << 0,
            BUMP_POOR = 0b10 << 0,
            BUMP_VPOOR = 0b11 << 0,
            CRACK_FAIR = 0b01 << 2,
            CRACK_POOR = 0b10 << 2,
            CRACK_VPOOR = 0b11 << 2,
            DEPRESSION_FAIR = 0b01 << 4,
            DEPRESSION_POOR = 0b10 << 4,
            DEPRESSION_VPOOR = 0b11 << 4,
            DISPLACEMENT_FAIR = 0b01 << 6,
            DISPLACEMENT_POOR = 0b10 << 6,
            DISPLACEMENT_VPOOR = 0b11 << 6,
            VEGETATION_FAIR = 0b01 << 8,
            VEGETATION_POOR = 0b10 << 8,
            VEGETATION_VPOOR = 0b11 << 8,
            UNEVEN_FAIR = 0b01 << 10,
            UNEVEN_POOR = 0b10 << 10,
            UNEVEN_VPOOR = 0b11 << 10,
            POTHOLE_FAIR = 0b01 << 12,
            POTHOLE_POOR = 0b10 << 12,
            POTHOLE_VPOOR = 0b11 << 12,
            PADDING_FAIR = 0b01 << 14,
            PADDING_POOR = 0b10 << 14,
            PADDING_VPOOR = 0b11 << 14,
        };

        // Define bitwise OR operator for Fault enum class
        inline Fault operator|(Fault lhs, Fault rhs) {
            return static_cast<Fault>(static_cast<uint16_t>(lhs) |
                                      static_cast<uint16_t>(rhs));
        }

        // Define bitwise OR assignment operator for Fault enum class
        inline Fault &operator|=(Fault &lhs, Fault rhs) {
            lhs = lhs | rhs;
            return lhs;
        }

        // Define bitwise AND operator for Fault enum class
        inline Fault operator&(Fault lhs, Fault rhs) {
            return static_cast<Fault>(static_cast<uint16_t>(lhs) &
                                      static_cast<uint16_t>(rhs));
        }

        struct Box {
            int x;
            int y;
            int w;
            int h;
            std::string image;
            Fault fault;

            std::string ToTsv(const std::string &) const;
        };

        using BoxArr = std::vector<Box>;

        struct ImgBox {
            std::string image;
            BoxArr boxes;

            std::string ToTsv(const std::string &) const;
        };

        using ImgBoxArr = std::vector<ImgBox>;

        struct ImgFaultCount {
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

            ImgFaultCount();

            ImgFaultCount(const ImgBox &ibox);

            std::string ToStr() const;
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
            void AddFault(const Fault &f);

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

        ImgBoxArr parseCsv(std::istream &);

        ImgBoxArr parseJson(std::istream &);

        void exportTsv(const std::string &, const std::string &,
                       std::ostream &);

        void drawImgBox(const ImgBox &, const std::string &,
                        const std::string &);

        void drawImgBoxes(const std::string &, const std::string &,
                          const std::string &, const std::string &);

        void exportStats(const std::string &, const std::string &);

        void printStats(const std::string &);

    } // namespace via

} // namespace fdt

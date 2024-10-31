#pragma once

#include <csv.hpp>
#include <iostream>
#include <vector>

namespace fdt {
    namespace ibox {

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
            POTHOLE_FAIR = 0b01 << 8,
            POTHOLE_POOR = 0b10 << 8,
            POTHOLE_VPOOR = 0b11 << 8,
            UNEVEN_FAIR = 0b01 << 10,
            UNEVEN_POOR = 0b10 << 10,
            UNEVEN_VPOOR = 0b11 << 10,
            VEGETATION_FAIR = 0b01 << 12,
            VEGETATION_POOR = 0b10 << 12,
            VEGETATION_VPOOR = 0b11 << 12,
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

            // Get the maximum severity level of the fault
            uint8_t MaxSeverity() const;

            std::string ToTsv(const std::string &) const;
        };

        struct ImgBox {
            std::string image;
            std::vector<Box> boxes;

            std::string ToTsv(const std::string &) const;

            void Draw(const std::string &, const std::string &) const;
        };

#ifdef GTEST_ACCESS
        std::vector<ImgBox> from_via_csv(std::istream &);

        std::vector<ImgBox> from_via_json(std::istream &);

        std::vector<ImgBox> from_csv_reader(csv::CSVReader &);
#endif
        std::vector<ImgBox> fromVia(const std::string &);

        std::vector<ImgBox> fromTsv(const std::string &);

        void toTsv(const std::vector<ibox::ImgBox> &, const std::string &,
                   std::ostream &);

        void drawBBox(const std::vector<ibox::ImgBox> &, const std::string &,
                      const std::string &);

    } // namespace ibox

} // namespace fdt

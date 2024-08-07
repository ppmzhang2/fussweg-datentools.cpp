#include "exif.hpp"
#include <ctime>
#include <gtest/gtest.h>

using namespace fdt::exif;

static constexpr double kEps = 0.0001;

// Test Attrs constructor with an invalid image path.
TEST(Attrs, InvalidPath) {
    const char *invalid_path = "invalid_path.jpg";
    try {
        auto attrs = Attrs(invalid_path);
    } catch (const std::runtime_error &err) {
        EXPECT_STREQ(err.what(), "Failed to open file: invalid_path.jpg");
    } catch (const std::exception &err) {
        FAIL() << "Unexpected exception: " << err.what();
    }
}

// Test Attrs constructor with an image with GPS data.
TEST(Attrs, ValidGPS) {
    const char *valid_gps_path = "tests/img/gps.jpg";
    const auto &attrs = Attrs(valid_gps_path);
    EXPECT_TRUE(*attrs.lat - 43.46744 < kEps);
    EXPECT_TRUE(*attrs.lon - 11.88512 < kEps);
}

TEST(Attrs, ToCsvFull) {
    static constexpr char kExpectedCsv[] =
        "image1.jpg\t4872\t5568\t19.473000\t2021-01-01T00:00:00\t-43."
        "530964\t172.676592\t1573867.695621\t5180169.055643";
    Attrs attrs;
    attrs.path = "folder1/folder2/image1.jpg";
    attrs.exif_ver = "48 50 50 49";
    attrs.desc = "DCIM\\101GOPRO\\G0018842.JPG";
    attrs.model = "GoPro HERO11 Black";
    attrs.height = 4872;
    attrs.width = 5568;
    attrs.lat = -43.530963599972225;
    attrs.lon = 172.6765924999722;
    attrs.altitude = 19.473;
    attrs.ts_gps = std::tm{0, 0, 0, 1, 0, 121, 0, 0, 0, 0, nullptr};

    std::ostringstream oss;
    attrs.ToCsv(oss);
    EXPECT_STREQ(oss.str().c_str(), kExpectedCsv);
}

TEST(Attrs, ToCsvEmpty) {
    static constexpr char kExpectedCsv[] =
        "image2.jpg\t4872\t\t19.473000\t2021-01-01T00:00:00\t-43."
        "530964\t172.676592\t1573867.695621\t5180169.055643";
    Attrs attrs;
    attrs.path = "folder1/folder2/image2.jpg";
    attrs.exif_ver = std::nullopt;
    attrs.desc = "";
    attrs.model = std::nullopt;
    attrs.height = 4872;
    attrs.width = std::nullopt;
    attrs.lat = -43.530963599972225;
    attrs.lon = 172.6765924999722;
    attrs.altitude = 19.473;
    attrs.ts_gps = std::tm{0, 0, 0, 1, 0, 121, 0, 0, 0, 0, nullptr};

    std::ostringstream oss;
    attrs.ToCsv(oss);
    EXPECT_STREQ(oss.str().c_str(), kExpectedCsv);
}

TEST(exif, ExportCsv) {
    static constexpr char kExpectedCsv[] =
        "image\theight\twidth\taltitude\ttimestamp\tlatitude\tlongitude\teasting"
        "\tno"
        "rthing\ngps.jpg\t480\t640\t0.000000\t2008-10-23T14:27:07\t43."
        "467447\t11."
        "885128\t7395536.307628\t51343528.675131\n";

    std::ostringstream oss;
    const char *valid_gps_path = "tests/img";
    exportCsv(valid_gps_path, oss);
    EXPECT_STREQ(oss.str().c_str(), kExpectedCsv);
}

#include "exif.hpp"
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

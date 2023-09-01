#include "exif.hpp"
#include <gtest/gtest.h>

/*
 * Test GetExifAttrs with an invalid image path.
 */
TEST(ExifErrorTests, TestGetExifAttrsInvalidPath) {
    const std::string invalid_path = "invalid_image_path.jpg";
    try {
        GetExifAttrs(invalid_path);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error &err) {
        EXPECT_EQ(
            err.what(),
            std::string("Failed to open the image or no Exif data found"));
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

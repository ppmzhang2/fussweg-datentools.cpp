#include "test_crs.cpp"
#include "test_exif.cpp"
#include "test_gis.cpp"
#include "test_ibox.cpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

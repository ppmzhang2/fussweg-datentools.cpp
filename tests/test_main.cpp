#include "test_annot.cpp"
#include "test_crs.cpp"
#include "test_exif.cpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

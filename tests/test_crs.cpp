#include "crs.hpp"
#include <gtest/gtest.h>

using namespace fdt::crs;

static constexpr double kLat1 = -41.2865, kLon1 = 174.7762;
static constexpr double kLat2 = -43.5321, Klon2 = 172.6362;
static constexpr double kLat3 = -36.8485, kLon3 = 176.2920;
static constexpr double kLat4 = -45.8742, kLon4 = 170.5036;
static constexpr double kEast1 = 1748735.55, kNorth1 = 5427916.47;
static constexpr double kEast2 = 1570604.41, kNorth2 = 5180029.37;
static constexpr double kEast3 = 1893535.04, kNorth3 = 5916873.49;
static constexpr double kEast4 = 1406262.16, kNorth4 = 4916899.03;

TEST(CRS, ToNZGD2000) {

    const auto [e1, n1] = ToNzgd2000(kLat1, kLon1);
    const auto [e2, n2] = ToNzgd2000(kLat2, Klon2);
    const auto [e3, n3] = ToNzgd2000(kLat3, kLon3);
    const auto [e4, n4] = ToNzgd2000(kLat4, kLon4);

    EXPECT_NEAR(e1, kEast1, 1e-2);
    EXPECT_NEAR(n1, kNorth1, 1e-2);
    EXPECT_NEAR(e2, kEast2, 1e-2);
    EXPECT_NEAR(n2, kNorth2, 1e-2);
    EXPECT_NEAR(e3, kEast3, 1e-2);
    EXPECT_NEAR(n3, kNorth3, 1e-2);
    EXPECT_NEAR(e4, kEast4, 1e-2);
    EXPECT_NEAR(n4, kNorth4, 1e-2);
}

TEST(CRS, FromNZGD2000) {

    const auto [lat1, lon1] = FromNzgd2000(kEast1, kNorth1);
    const auto [lat2, lon2] = FromNzgd2000(kEast2, kNorth2);
    const auto [lat3, lon3] = FromNzgd2000(kEast3, kNorth3);
    const auto [lat4, lon4] = FromNzgd2000(kEast4, kNorth4);

    EXPECT_NEAR(lat1, kLat1, 1e-2);
    EXPECT_NEAR(lon1, kLon1, 1e-2);
    EXPECT_NEAR(lat2, kLat2, 1e-2);
    EXPECT_NEAR(lon2, Klon2, 1e-2);
    EXPECT_NEAR(lat3, kLat3, 1e-2);
    EXPECT_NEAR(lon3, kLon3, 1e-2);
    EXPECT_NEAR(lat4, kLat4, 1e-2);
    EXPECT_NEAR(lon4, kLon4, 1e-2);
}

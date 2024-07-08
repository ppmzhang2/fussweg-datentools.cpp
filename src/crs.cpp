#include "crs.hpp"

#include <stdexcept>

namespace {
    // Constants for the NZTM2000 projection
    static constexpr double kFalseEasting = 1600000;
    static constexpr double kFalseNorthing = 10000000;
    static constexpr double kK = 0.9996;

    // Longitude of the central meridian; for NZTM2000 it is 173 degrees east.
    static constexpr double kLon = DEG2RAD(173);

    // Semi-major axis of the Earth, the longest radius of the ellipsoid
    static constexpr double kAlpha = 6378137.0;

    // Semi-minor axis of the Earth, the shortest radius of the ellipsoid
    static constexpr double kBeta = 6356752.314245179;

    // Flattening (F): measure of the compression of a sphere (in this context,
    // a planet or moon) along its axis, resulting in an ellipsoid with a
    // shorter axis from pole to pole than at the equator.
    // It quantifies how much an ellipsoid deviates from a perfect sphere.
    static constexpr double kF = (kAlpha - kBeta) / kAlpha;

    // First Eccentricity Squared (e^2): a measure of how much the ellipsoid
    // deviates from a sphere, calculated from the flattening of the ellipsoid.
    // Formula: 2 * f - f^2 = (alpha^2 - beta^2) / alpha^2
    static constexpr double kEE1st = 2 * kF - kF * kF;

    // Correction term coefficients for the meridional arc
    static constexpr double A0 = 1 - (kEE1st / 4) - (kEE1st * kEE1st * 3 / 64) -
                                 (kEE1st * kEE1st * kEE1st * 5 / 256);
    static constexpr double A2 =
        (3 / 8.0) * (kEE1st + (kEE1st * kEE1st / 4) +
                     (kEE1st * kEE1st * kEE1st * 15 / 128));
    static constexpr double A4 =
        (15 / 256.0) * (kEE1st * kEE1st + (kEE1st * kEE1st * kEE1st * 3 / 4));
    static constexpr double A6 = (35.0 / 3072.0) * kEE1st * kEE1st * kEE1st;

} // namespace

// Validation functions
static inline constexpr void validate_latitude(const double latitude) {
    if (latitude < -90 || latitude > 90) {
        throw std::invalid_argument(
            "Latitude must be between -90 and 90 degrees.");
    }
}
static inline constexpr void validate_longitude(const double longitude) {
    if (longitude < -180 || longitude > 180) {
        throw std::invalid_argument(
            "Longitude must be between -180 and 180 degrees.");
    }
}

// Radius of Curvature in the Prime Vertical at a given latitude (N)
//
// It represents the radius of the GREAT CIRCLE that best fits the ellipsoid at
// that latitude in the plane, perpendicular to the meridian (north-south
// direction).
//
// - Formula: alpha / sqrt(1 - e^2 * sin^2(lat))
//
// - Properties:
//   - measures how much the ellipsoid flattens at the poles and bulges at the
//     equator by providing the curvature radius at any given latitude.
//   - serves as a scaling factor for the latitude to adjust the distance
//     calculations to the actual surface curve of the ellipsoid.
//   - In practical terms, N adjusts the easting and northing calculations to
//     be more accurate according to the true distances along the surface of
//     the Earth.
//
// @param sin_lat: sine of the latitude in radian
static inline constexpr double radius_curvature(const double sin_lat) {
    return kAlpha / sqrt(1 - kEE1st * sin_lat * sin_lat);
}

// Adjusted Longitude Difference (A).
//
// The radian difference between the given longitude and the central meridian,
// scaled by the cosine of the latitude, accounting for the shrinking of the
// radius of cross-section circles as one moves away from the equator.
//
// ```plaintext
//                 **************
//             **********************
//          ****************************
//       ***********      +=======+********
//      ********          |      /| ********
//    ********            |     / |   ********
//   *******              |    /  |     *******
//  *******               |   /   |      *******
//  ******                |  /    |       ******
// ******                 | /     |        ******
// ******                 |/ lat  |        ******
// ******                 .----------------******
// ```
//
// As illustrated in the diagram above, when calculating the easting (arc
// length from west to east), the radius of the cross-section circle (indicated
// with `=`) should be scaled down from the ellipsoid's radius (calculated with
// `radius_curvature`) by the cosine of the latitude as the latitude increases.
//
// The scale factor is applied to the delta longitude to facilitate further
// corrections.
//
// @param lon: longitude in radians
// @param cos_lat: cosine of the latitude in radian
static inline constexpr double delta_long_adj(const double lon,
                                              const double cos_lat) {
    return (lon - kLon) * cos_lat;
}

// Meridional Arc (M)
//
// The northward distance from the equator to the latitude given. The correction
// items account for the curvature of the ellipsoidal earth.
//
// Formula:
//
// - alpha * (A0 * lat - A2 * sin(2 lat) + A4 * sin(4 lat) - A6 * sin(6 lat))
//
// where coefficients A0, A2, A4, and A6 are derived from the eccentricity of
// the ellipsoid:
//
// - A0 = 1 - (1 / 4 * e^2) - (3 / 64 * e^4) - (5 / 256 * e^6)
// - A2 = (3 / 8) * (e^2 + (1 / 4 * e^4) + (15 / 128 * e^6))
// - A4 = (15 / 256) * (e^4 + (3 / 4 * e^6))
// - A6 = (35 / 3072) * e^6
//
// @param lat: latitude in radians
static inline constexpr double arc_meridian(const double lat) {
    return kAlpha * (A0 * lat - A2 * sin(2 * lat) + A4 * sin(4 * lat) -
                     A6 * sin(6 * lat));
}

// Second Eccentricity Squared Adjusted (C).
//
// Formula: e^2 / (1 - e^2) * cos^2(lat)
//
// TODO: reasoning
//
// @param cos_lat: cosine of the latitude in radian
static inline constexpr double ee_2nd_adj(const double cos_lat) {
    return (kEE1st / (1 - kEE1st)) * cos_lat * cos_lat;
}

// Convert geographic coordinates from WGS84 to NZGD2000, using the Transverse
// Mercator projection.
//
// Formulas with only primary terms:
//
// - Northing (y): y = M
// - Easting (x): x = N * A
//
// where
//
//   - Meridional Arc (M): see `arc_meridian`
//   - Radius of Curvature in the Prime Vertical at a given latitude (N): see
//     `radius_curvature`
//   - Adjusted Longitude Difference (A): see `delta_long_adj`
//
// Formula with Corrections Terms:
//
// - Easting: x = N * (A + Cubic Correction + Quintic Correction)
//   - Cubic Correction: Primary Ellipsoidal Correction
//     - Formula: (1 - T^2 + C) * A^3 / 6
//     - Purpose: Corrects for the ellipsoidal shape of the earth, taking into
//       account the squared tangent of latitude `T^2` and the square of the
//       first eccentricity `C`, which adjusts for the flattening effects near
//       the poles.
//   - Quintic Correction: Higher-Order Ellipsoidal Correction
//     - Formula: (5 - 18 * T^2 + T^4 + 72 * C - 58 * e^2) * A^5 / 120
//     - Purpose: Further refinement based on higher-order terms of latitude
//       and eccentricity, ensuring accuracy over larger areas and extreme
//       latitudinal positions.
//
// - Northing: y = M + N * T * (Quadratic + Quartic + Sextic Correction)
//   - Quadratic Correction: Basic Curvature Correction
//     - Formula: A^2 / 2
//     - Purpose: Adjusts for basic curvature effects due to the projection of
//       the spherical coordinates onto a plane, particularly noticeable at
//       higher latitudes.
//   - Quartic Correction: Moderate Ellipsoidal Correction
//     - Formula: (5 - T^2 + 9 * C + 4 * C^2) * A^4 / 24
//     - Purpose: Corrects for moderate distortions due to the ellipsoid shape,
//       incorporating terms for tangent and eccentricity squared.
//   - Sextic Correction: Advanced Ellipsoidal Correction
//     - Formula: (61 - 58 * T^2 + T^4 + 600 * C - 330 * e^2) * A^6 / 720
//     - Purpose: A higher-order correction for more significant distortions,
//       integrating complex interactions of latitude, flattening, and
//       eccentricity for precise mapping across extensive geographical areas.
//
// @param lat: latitude in degrees
// @param lon: longitude in degrees
//
// @return: a tuple containing the easting and northing coordinates in meters
//          on the NZGD2000 coordinate system.
//
// @throws: std::invalid_argument if the latitude or longitude is out of range.
std::tuple<double, double> fdt::crs::ToNzgd2000(const double lat,
                                                const double lon) {
    validate_latitude(lat);
    validate_longitude(lon);

    // Convert latitude / longitude to radians
    const double lat_ = DEG2RAD(lat);
    const double lon_ = DEG2RAD(lon);

    // trigonometric constants used later
    const double sin_lat = sin(lat_);
    const double cos_lat = cos(lat_);
    const double t = tan(lat_);

    // Key terms
    const double m = arc_meridian(lat_);
    const double n = radius_curvature(sin_lat);
    const double a = delta_long_adj(lon_, cos_lat);
    const double c = ee_2nd_adj(cos_lat);

    // Calculate easting and northing
    const double x =
        n * (a + (1 - t * t + c) * a * a * a / 6 +
             (5 - 18 * t * t + t * t * t * t + 72 * c - 58 * kEE1st) * a * a *
                 a * a * a / 120);
    const double y =
        m +
        n * t *
            (a * a / 2 + (5 - t * t + 9 * c + 4 * c * c) * a * a * a * a / 24 +
             (61 - 58 * t * t + t * t * t * t + 600 * c - 330 * kEE1st) * a *
                 a * a * a * a * a / 720);

    // Apply the scale factor and the false easting/northing
    const double easting = x * kK + kFalseEasting;
    const double northing = y * kK + kFalseNorthing;

    return std::make_tuple(easting, northing);
}

// Convert geographic coordinates from NZGD2000 to WGS84, using the Transverse
// Mercator projection.
//
// See the comments in `ToNzgd2000` for an explanation.
std::tuple<double, double> fdt::crs::FromNzgd2000(const double easting,
                                                  const double northing) {
    // Remove the false easting and northing
    const double x = (easting - kFalseEasting) / kK;
    const double y = (northing - kFalseNorthing) / kK;

    // Calculate the footpoint latitude
    const double m = y;
    const double mu = m / (kAlpha * A0);

    const double e1 = (1 - sqrt(1 - kEE1st)) / (1 + sqrt(1 - kEE1st));

    const double phi1 =
        mu + (3 * e1 / 2 - 27 * e1 * e1 * e1 / 32) * sin(2 * mu) +
        (21 * e1 * e1 / 16 - 55 * e1 * e1 * e1 * e1 / 32) * sin(4 * mu) +
        (151 * e1 * e1 * e1 / 96) * sin(6 * mu) +
        (1097 * e1 * e1 * e1 * e1 / 512) * sin(8 * mu);

    // trigonometric constants used later
    const double sin_phi1 = sin(phi1);
    const double cos_phi1 = cos(phi1);
    const double tan_phi1 = tan(phi1);

    const double n1 = radius_curvature(sin_phi1);
    const double t1 = tan_phi1 * tan_phi1;
    const double c1 = ee_2nd_adj(cos_phi1);
    const double r1 =
        kAlpha * (1 - kEE1st) / pow(1 - kEE1st * sin_phi1 * sin_phi1, 1.5);
    const double d = x / n1;

    // Calculate latitude
    const double latitude =
        phi1 - (n1 * tan_phi1 / r1) *
                   (d * d / 2 -
                    (5 + 3 * t1 + 10 * c1 - 4 * c1 * c1 - 9 * kEE1st) * d * d *
                        d * d / 24 +
                    (61 + 90 * t1 + 298 * c1 + 45 * t1 * t1 - 252 * kEE1st -
                     3 * c1 * c1) *
                        d * d * d * d * d * d / 720);

    // Calculate longitude
    const double longitude =
        kLon +
        (d - (1 + 2 * t1 + c1) * d * d * d / 6 +
         (5 - 2 * c1 + 28 * t1 - 3 * c1 * c1 + 8 * kEE1st + 24 * t1 * t1) * d *
             d * d * d * d / 120) /
            cos_phi1;

    return std::make_tuple(RAD2DEG(latitude), RAD2DEG(longitude));
}

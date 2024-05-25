#include "crs.hpp"

#include <stdexcept>

namespace {
    static constexpr double kFalseEasting = 1600000;
    static constexpr double kFalseNorthing = 10000000;
    static constexpr double kK = 0.9996;
    // Semi-major axis for GRS 80
    static constexpr double kAlpha = 6378137.0;
    // Semi-minor axis for GRS 80
    static constexpr double kBeta = 6356752.314245179;
    // Flattening for GRS 80
    static constexpr double kF = (kAlpha - kBeta) / kAlpha;
    // Eccentricity squared
    static constexpr double kE2 = 2 * kF - kF * kF;
    static constexpr double kE4 = kE2 * kE2;
    static constexpr double kE6 = kE4 * kE2;
    // Central meridian in radians
    static constexpr double kLambda0 = DEG2RAD(173);

    // Calculate coefficients for the meridional arc
    static constexpr double A0 =
        1 - (kE2 / 4) - (kE4 * 3 / 64) - (kE6 * 5 / 256);
    static constexpr double A2 =
        (3.0 / 8.0) * (kE2 + (kE4 / 4) + (kE6 * 15 / 128));
    static constexpr double A4 = (15.0 / 256.0) * (kE4 + (kE6 * 3 / 4));
    static constexpr double A6 = (35.0 / 3072.0) * kE6;

    static inline constexpr void validate_latitude(double latitude) {
        if (latitude < -90 || latitude > 90) {
            throw std::invalid_argument(
                "Latitude must be between -90 and 90 degrees.");
        }
    }
    static inline constexpr void validate_longitude(double longitude) {
        if (longitude < -180 || longitude > 180) {
            throw std::invalid_argument(
                "Longitude must be between -180 and 180 degrees.");
        }
    }
} // namespace

// Convert geographic coordinates from WGS84 to NZGD2000, using the Transverse
// Mercator projection.
//
// Key Terms:
//
// - latitude in radians (phi): geographic latitude in radians.
//
// - longitude in radians (lambda): geographic longitude in radians.
//
// - longitude of the central meridian (lambda0): for NZTM2000, the central
//   meridian is 173 degrees east.
//
// - semi-major axis (alpha): the semi-major axis of the ellipsoid, the
//   longest radius of the ellipsoid.
//
// - flattening (f): a measure of the compression of a sphere (in this
//   context, a planet or moon) along its axis, resulting in an ellipsoid with
//   a shorter axis from pole to pole than at the equator. It quantifies how
//   much an ellipsoid deviates from being a perfect sphere.
//   - formula: (alpha - beta) / alpha
//     - alpha: semi-major axis
//     - beta: semi-minor axis
//   - purpose
//     - Shape Approximation: Flattening is critical in accurately describing
//       the Earth's shape. The Earth is not a perfect sphere but an oblate
//       spheroid, meaning it is flattened at the poles due to its rotation.
//       This flattening must be accurately represented in models used for
//       navigation, surveying, and mapping.
//     - Distortion Minimization in Mapping: In map projections, understanding
//       the flattening of the Earth allows cartographers to minimize
//       distortions when converting the three-dimensional surface of the
//       Earth to two-dimensional maps. This is particularly important for
//       ensuring accuracy in areas of critical navigation, land ownership
//       mapping, and other applications where spatial precision is mandatory.
//
// - eccentricity squared (e^2): the square of the eccentricity of the
//   ellipsoid.
//   - formula: 2 * f - f^2 = (alpha^2 - beta^2) / alpha^2
//   - purpose
//     - Ellipsoidal Shape Modeling: Eccentricity squared is critical for
//       accurately modeling the ellipsoidal shape of the Earth. It directly
//       influences calculations related to the Earth's curvature in different
//       geographic coordinate systems and geodetic calculations.
//     - Map Projections: In map projections, `e^2` is crucial for adjusting
//       the scaling and distortion that occur when the Earth's surface is
//       represented on a flat plane, ensuring that these representations are
//       as accurate as possible.
//
// - meridional arc (M):
//
//   alpha * (A0 * phi - A2 * sin(2 phi) + A4 * sin(4 phi) - A6 * sin(6 phi))
//
//   where coefficients A0, A2, A4, and A6 are derived from the eccentricity
//   of the ellipsoid:
//
//   - A0 = 1 - (1 / 4 * e^2) - (3 / 64 * e^4) - (5 / 256 * e^6)
//   - A2 = (3 / 8) * (e^2 + (1 / 4 * e^4) + (15 / 128 * e^6))
//   - A4 = (15 / 256) * (e^4 + (3 / 4 * e^6))
//   - A6 = (35 / 3072) * e^6
//
// - radius of curvature in the prime vertical (N): the prime vertical is the
//   east-west oriented plane that intersects the ellipsoid at a given
//   latitude and is perpendicular to the meridian plane.
//
//   - Formula: alpha / sqrt(1 - e^2 * sin^2(phi))
//   - Purpose
//     - measures how much the ellipsoid flattens at the poles and bulges at
//       the equator by providing the curvature radius at any given latitude.
//     - serves as a scaling factor for the latitude to adjust the distance
//       calculations to the actual surface curve of the ellipsoid.
//     - In practical terms, N adjusts the easting and northing calculations
//       to be more accurate according to the true distances along the surface
//       of the Earth.
//
// - tangent of latitude (T):
//
//   - Formula: tan(phi)
//   - Purpose
//     - directly impacts the calculation of the northing coordinate in the
//       projection, particularly influencing the quadratic and higher-order
//       terms.
//     - The tangent function increases dramatically as latitude approaches
//       +/-90 degrees (the poles), which is critical because the projection
//       needs to account for greater distortions as one moves away from the
//       equator.
//     - also represents the slope of the tangent line to the meridian at
//       latitude phi, relating to how steeply the Earth curves away at that
//       latitude.
//
// - second eccentricity squared (C):
//
//   - Formula: (e^2 / (1 - e^2)) * cos^2(phi)
//   - Purpose
//     - accounts for the ellipsoidal shape's effect on meridian convergence,
//       modifying how distances calculated along meridians and parallels vary
//       with latitude.
//     - adjusts the distance calculations to factor in the flattening effect
//       of the ellipsoid as one moves north or south from the equator. The
//       Earth is not a perfect sphere, so the distances along the parallels
//       (east-west direction) and meridians (north-south direction) do not
//       uniformly scale; `C` helps correct for these variances.
//     - primarily affects the higher-order corrections in the easting and
//       northing formulas, ensuring that these adjustments become more
//       significant as one moves further from the equator and as the effects
//       of the Earth's flattening become more pronounced.
//
// - adjusted longitude difference (A): adjusted longitudinal difference
//   between a given point and the central meridian, factoring in the
//   latitude. It essentially adjusts for the convergence of meridians toward
//   the poles.
//
//   - formula: (lambda - lambda0) * cos(phi)
//   - purpose
//     - Eastward Displacement: `A` quantifies the eastward displacement from
//       the central meridian but scales this displacement by the cosine of
//       the latitude. This scaling is crucial because as you move away from
//       the equator toward the poles, the physical distance represented by a
//       degree of longitude decreases. `A` compensates for this by reducing
//       the eastward displacement as the cosine of the latitude decreases
//       (i.e., as one moves closer to the poles).
//     - Projection Scaling: By multiplying the longitudinal difference by the
//       cosine of the latitude, `A` helps in maintaining accurate scale
//       across the map. In a cylindrical projection like the Transverse
//       Mercator, if this scaling were not applied, areas farther from the
//       equator would appear disproportionately stretched in the east-west
//       direction relative to their actual ground size.
//     - Reduces Distortion: This term is critical in reducing distortion in
//       the projected coordinates. By adjusting for the latitude's influence
//       on the distance covered by a degree of longitude, `A` helps ensure
//       that the projection retains more accurate geographical proportions,
//       which is particularly important in high-accuracy mapping applications
//       like cadastral and navigation systems.
//     - Simplifies Higher-Order Corrections: `A` is used in the calculation
//       of higher-order terms for both easting and northing, which further
//       correct for the curvature and the elliptical shape of the earth.
//       These corrections are essential for minimizing errors in the
//       projection, especially over larger areas or regions with significant
//       relief.
//
// - Easting (x):
//
//   - Formula: N * (Linear Term + Cubic Correction + Quintic Correction)
//     - Linear Term: Basic Eastward Displacement
//       - Formula: A
//       - Purpose: represents the basic eastward displacement from the
//         central meridian, adjusted by the cosine of latitude to account for
//         convergence of meridians towards the poles.
//     - Cubic Correction: Primary Ellipsoidal Correction
//       - Formula: (1 - T^2 + C) * A^3 / 6
//       - Purpose: Corrects for the ellipsoidal shape of the earth, taking
//         into account the squared tangent of latitude `T^2` and the square
//         of the first eccentricity `C`, which adjusts for the flattening
//         effects near the poles.
//     - Quintic Correction: Higher-Order Ellipsoidal Correction
//       - Formula: (5 - 18 * T^2 + T^4 + 72 * C - 58 * e^2) * A^5 / 120
//       - Purpose: Further refinement based on higher-order terms of latitude
//         and eccentricity, ensuring accuracy over larger areas and extreme
//         latitudinal positions.
//
// - Northing (y): Incorporates the meridional arc and additional terms that
//   correct for north-south displacement and the curvature effects.
//
//   - Formula:
//     Meridional Arc + N * T * (Quadratic + Quartic + Sextic Correction)
//     - Meridional Arc (M): Meridional Distance
//       - Purpose: Represents the northward distance from the equator to the
//         latitude `phi` accounting for the curvature of the ellipsoidal
//         earth.
//     - Quadratic Correction: Basic Curvature Correction
//       - Formula: A^2 / 2
//       - Purpose: Adjusts for basic curvature effects due to the projection
//         of the spherical coordinates onto a plane, particularly noticeable
//         at higher latitudes.
//     - Quartic Correction: Moderate Ellipsoidal Correction
//       - Formula: (5 - T^2 + 9 * C + 4 * C^2) * A^4 / 24
//       - Purpose: Corrects for moderate distortions due to the ellipsoid
//         shape, incorporating terms for tangent and eccentricity squared.
//     - Sextic Correction: Advanced Ellipsoidal Correction
//       - Formula: (61 - 58 * T^2 + T^4 + 600 * C - 330 * e^2) * A^6 / 720
//       - Purpose: A higher-order correction for more significant
//         distortions, integrating complex interactions of latitude,
//         flattening, and eccentricity for precise mapping across extensive
//         geographical areas.
std::tuple<double, double> const fdt::crs::ToNzgd2000(double lat, double lon) {
    validate_latitude(lat);
    validate_longitude(lon);

    // Convert latitude / longitude to radians
    const double phi = DEG2RAD(lat);
    const double lambda = DEG2RAD(lon);

    // Calculate the meridional arc
    const double m = kAlpha * (A0 * phi - A2 * sin(2 * phi) +
                               A4 * sin(4 * phi) - A6 * sin(6 * phi));

    // Calculate the radius of curvature in the prime vertical
    const double n = kAlpha / sqrt(1 - kE2 * sin(phi) * sin(phi));
    const double t = tan(phi);
    const double c = (kE2 / (1 - kE2)) * cos(phi) * cos(phi);
    const double a = (lambda - kLambda0) * cos(phi);

    // Calculate easting and northing
    const double x = n * (a + (1 - t * t + c) * a * a * a / 6 +
                          (5 - 18 * t * t + t * t * t * t + 72 * c - 58 * kE2) *
                              a * a * a * a * a / 120);
    const double y =
        m +
        n * tan(phi) *
            (a * a / 2 + (5 - t * t + 9 * c + 4 * c * c) * a * a * a * a / 24 +
             (61 - 58 * t * t + t * t * t * t + 600 * c - 330 * kE2) * a * a *
                 a * a * a * a / 720);

    // Apply the scale factor and the false easting/northing
    const double easting = x * kK + kFalseEasting;
    const double northing = y * kK + kFalseNorthing;

    return std::make_tuple(easting, northing);
}

// Convert geographic coordinates from NZGD2000 to WGS84, using the Transverse
// Mercator projection.
//
// See the comments in `ToNzgd2000` for an explanation.
std::tuple<double, double> const fdt::crs::FromNzgd2000(double easting,
                                                        double northing) {
    // Remove the false easting and northing
    const double x = (easting - kFalseEasting) / kK;
    const double y = (northing - kFalseNorthing) / kK;

    // Calculate the footpoint latitude
    const double m = y;
    const double mu = m / (kAlpha * A0);

    const double e1 = (1 - sqrt(1 - kE2)) / (1 + sqrt(1 - kE2));

    const double phi1 =
        mu + (3 * e1 / 2 - 27 * e1 * e1 * e1 / 32) * sin(2 * mu) +
        (21 * e1 * e1 / 16 - 55 * e1 * e1 * e1 * e1 / 32) * sin(4 * mu) +
        (151 * e1 * e1 * e1 / 96) * sin(6 * mu) +
        (1097 * e1 * e1 * e1 * e1 / 512) * sin(8 * mu);

    const double n1 = kAlpha / sqrt(1 - kE2 * sin(phi1) * sin(phi1));
    const double t1 = tan(phi1) * tan(phi1);
    const double c1 = kE2 / (1 - kE2) * cos(phi1) * cos(phi1);
    const double r1 =
        kAlpha * (1 - kE2) / pow(1 - kE2 * sin(phi1) * sin(phi1), 1.5);
    const double d = x / n1;

    // Calculate latitude
    const double latitude =
        phi1 - (n1 * tan(phi1) / r1) *
                   (d * d / 2 -
                    (5 + 3 * t1 + 10 * c1 - 4 * c1 * c1 - 9 * kE2) * d * d * d *
                        d / 24 +
                    (61 + 90 * t1 + 298 * c1 + 45 * t1 * t1 - 252 * kE2 -
                     3 * c1 * c1) *
                        d * d * d * d * d * d / 720);

    // Calculate longitude
    const double longitude =
        kLambda0 +
        (d - (1 + 2 * t1 + c1) * d * d * d / 6 +
         (5 - 2 * c1 + 28 * t1 - 3 * c1 * c1 + 8 * kE2 + 24 * t1 * t1) * d * d *
             d * d * d / 120) /
            cos(phi1);

    return std::make_tuple(RAD2DEG(latitude), RAD2DEG(longitude));
}

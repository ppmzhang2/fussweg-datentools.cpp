#include "gis.hpp"
#include <gtest/gtest.h>

static constexpr char kGeoJson[] = R"({
    "crs": {
        "type": "name",
        "properties": { "name": "urn:ogc:def:crs:EPSG::2193" }
    },
    "features": [
        {
            "identifier": {
                "EntityId": "fc6e0039-06ad-43f6-94cb-c703edfb63b8",
                "AssetId": "24143",
                "SecondaryId": null,
                "AssetId3": null,
                "AssetId4": null,
                "AssetId5": null,
                "TableName": "footpath",
                "CoreIdentifier": null,
                "Identifier": "fc6e0039-06ad-43f6-94cb-c703edfb63b8",
                "AssetIds": "24143",
                "AssetIdList": ["24143"]
            },
            "properties": {
                "road_id": "BLENHEIM RD1/ACCESS TO RLY LINE",
                "start_m": 170,
                "end_m": null,
                "side": "Unknown",
                "footpath_position": "Accessway Joins another road",
                "length_m": 183,
                "width": 2.6,
                "area": 475.8,
                "total_area": 475.8,
                "footpath_surf_mat": "Asphaltic concrete black",
                "age": 15,
                "purpose": "Both",
                "pedestrian_use": "Medium",
                "bicycle_use": "Medium",
                "scooter_use": null,
                "id": "fc6e0039-06ad-43f6-94cb-c703edfb63b8",
                "is_converted": false
            },
            "geometry": {
                "coordinates": [1568455.8784, 5179521.0882],
                "shapeType": "Point",
                "type": "Point"
            },
            "type": "Feature"
        },
        {
            "identifier": {
                "EntityId": "fb9baee1-1d85-442d-b691-48fddf7ad861",
                "AssetId": "27733",
                "SecondaryId": null,
                "AssetId3": null,
                "AssetId4": null,
                "AssetId5": null,
                "TableName": "footpath",
                "CoreIdentifier": null,
                "Identifier": "fb9baee1-1d85-442d-b691-48fddf7ad861",
                "AssetIds": "27733",
                "AssetIdList": ["27733"]
            },
            "properties": {
                "road_id": "COURTENAY ST",
                "start_m": 190,
                "end_m": 215,
                "side": "Right",
                "footpath_position": "Accessway Joins another road",
                "length_m": 42,
                "width": 1.5,
                "area": 63.0,
                "total_area": 63.0,
                "footpath_surf_mat": "Asphaltic concrete black",
                "age": 44,
                "purpose": "Footpath",
                "pedestrian_use": null,
                "bicycle_use": null,
                "scooter_use": null,
                "id": "fb9baee1-1d85-442d-b691-48fddf7ad861",
                "is_converted": false
            },
            "geometry": {
                "coordinates": [
                    [1570453.1775866507, 5182792.0856389459],
                    [1570463.6855638945, 5182793.8602143824],
                    [1570471.708698421, 5182793.6793808965],
                    [1570479.6129959677, 5182795.8746112091],
                    [1570483.5713081418, 5182795.567898361],
                    [1570490.7861228182, 5182794.3032251243],
                    [1570492.2458392591, 5182795.1738442136],
                    [1570493.4816331379, 5182797.6638869913]
                ],
                "shapeType": "LineString",
                "type": "LineString"
            },
            "type": "Feature"
        },
        {
            "identifier": {
                "EntityId": "7c0db807-1d97-4a2a-880a-d6faa32ecfac",
                "AssetId": "23295",
                "SecondaryId": null,
                "AssetId3": null,
                "AssetId4": null,
                "AssetId5": null,
                "TableName": "footpath",
                "CoreIdentifier": null,
                "Identifier": "7c0db807-1d97-4a2a-880a-d6faa32ecfac",
                "AssetIds": "23295",
                "AssetIdList": ["23295"]
            },
            "properties": {
                "road_id": "Venilla Polygon",
                "start_m": 4,
                "end_m": 67,
                "side": "Right",
                "footpath_position": "Kerb",
                "length_m": 63,
                "width": 1.4,
                "area": 88.2,
                "total_area": 88.2,
                "footpath_surf_mat": "Asphaltic concrete black",
                "age": 17,
                "purpose": "Footpath",
                "pedestrian_use": "Low/medium",
                "bicycle_use": null,
                "scooter_use": null,
                "id": "7c0db807-1d97-4a2a-880a-d6faa32ecfac",
                "is_converted": false
            },
            "geometry": {
                "coordinates": [
                    [
                        [1568287.2462070503, 5179502.6947125858],
                        [1568291.926131285, 5179501.56489106],
                        [1568299.9770336186, 5179500.4907283979],
                        [1568305.6780365957, 5179500.7179030888],
                        [1568356.6228534346, 5179506.519053583],
                        [1568356.7812495716, 5179505.12804292],
                        [1568305.7852512563, 5179499.3210643269],
                        [1568299.9118470573, 5179499.0870197574],
                        [1568291.6685838094, 5179500.1868474819],
                        [1568286.9176595672, 5179501.3338097418],
                        [1568287.2462070503, 5179502.6947125858]
                    ]
                ],
                "shapeType": "Polygon",
                "type": "Polygon"
            },
            "type": "Feature"
        },
        {
            "identifier": {
                "EntityId": "33f00889-b86e-4300-a069-428f1ae78531",
                "AssetId": "28434",
                "SecondaryId": null,
                "AssetId3": null,
                "AssetId4": null,
                "AssetId5": null,
                "TableName": "footpath",
                "CoreIdentifier": null,
                "Identifier": "33f00889-b86e-4300-a069-428f1ae78531",
                "AssetIds": "28434",
                "AssetIdList": ["28434"]
            },
            "properties": {
                "road_id": "Polygon with One Hole",
                "start_m": 1,
                "end_m": 173,
                "side": "Left",
                "footpath_position": "Whole Width",
                "length_m": 170,
                "width": 4.0,
                "area": 680.0,
                "total_area": 680.0,
                "footpath_surf_mat": "Asphaltic concrete black",
                "age": 2,
                "purpose": "Both",
                "pedestrian_use": "Low/medium",
                "bicycle_use": "Low/medium",
                "scooter_use": null,
                "id": "33f00889-b86e-4300-a069-428f1ae78531",
                "is_converted": false
            },
            "geometry": {
                "coordinates": [
                    [
                        [1740568.898, 5419812.500],
                        [1740578.998, 5419812.500],
                        [1740578.998, 5419822.608],
                        [1740568.898, 5419822.608],
                        [1740568.898, 5419812.500]
                    ],
                    [
                        [1740571.950, 5419815.555],
                        [1740575.945, 5419815.555],
                        [1740575.945, 5419819.552],
                        [1740571.950, 5419819.552],
                        [1740571.950, 5419815.555]
                    ]
                ],
                "shapeType": "Polygon",
                "type": "Polygon"
            },
            "type": "Feature"
        },
        {
            "identifier": {
                "EntityId": "36b1450b-f532-4a9b-9f6d-de52130cf741",
                "AssetId": "18950",
                "SecondaryId": null,
                "AssetId3": null,
                "AssetId4": null,
                "AssetId5": null,
                "TableName": "footpath",
                "CoreIdentifier": null,
                "Identifier": "36b1450b-f532-4a9b-9f6d-de52130cf741",
                "AssetIds": "18950",
                "AssetIdList": ["18950"]
            },
            "properties": {
                "road_id": "Polygon with Multiple Holes",
                "start_m": 769,
                "end_m": 777,
                "side": "Right",
                "footpath_position": "Middle",
                "length_m": 8,
                "width": 2.3,
                "area": 18.4,
                "total_area": 18.4,
                "footpath_surf_mat": "Wood",
                "age": 24,
                "purpose": "Footpath",
                "pedestrian_use": "Low/medium",
                "bicycle_use": null,
                "scooter_use": null,
                "id": "36b1450b-f532-4a9b-9f6d-de52130cf741",
                "is_converted": false
            },
            "geometry": {
                "coordinates": [
                    [
                        [1741756.244, 5420752.589],
                        [1741856.311, 5420752.589],
                        [1741856.311, 5420852.676],
                        [1741756.244, 5420852.676],
                        [1741756.244, 5420752.589]
                    ],
                    [
                        [1741771.259, 5420772.604],
                        [1741841.296, 5420772.604],
                        [1741841.296, 5420832.661],
                        [1741771.259, 5420832.661],
                        [1741771.259, 5420772.604]
                    ],
                    [
                        [1741786.274, 5420787.619],
                        [1741831.291, 5420787.619],
                        [1741831.291, 5420817.648],
                        [1741786.274, 5420817.648],
                        [1741786.274, 5420787.619]
                    ]
                ],
                "shapeType": "Polygon",
                "type": "Polygon"
            },
            "type": "Feature"
        },
        {
            "identifier": {
                "EntityId": "9d9a8926-958e-472c-b4a5-861912d6709d",
                "AssetId": "16529",
                "SecondaryId": null,
                "AssetId3": null,
                "AssetId4": null,
                "AssetId5": null,
                "TableName": "footpath",
                "CoreIdentifier": null,
                "Identifier": "9d9a8926-958e-472c-b4a5-861912d6709d",
                "AssetIds": "16529",
                "AssetIdList": ["16529"]
            },
            "properties": {
                "road_id": "AOTEA TCE",
                "start_m": 475,
                "end_m": 729,
                "side": "Right",
                "footpath_position": "Kerb",
                "length_m": 254,
                "width": 1.5,
                "area": 381.0,
                "total_area": 381.0,
                "footpath_surf_mat": "Asphaltic concrete black",
                "age": 44,
                "purpose": "Footpath",
                "pedestrian_use": "Low/medium",
                "bicycle_use": null,
                "scooter_use": null,
                "id": "9d9a8926-958e-472c-b4a5-861912d6709d",
                "is_converted": false
            },
            "geometry": {
                "coordinates": [
                    [
                        [
                            [1572223.199, 5176019.291],
                            [1572223.200, 5176019.376],
                            [1572223.161, 5176019.331],
                            [1572223.199, 5176019.291]
                        ]
                    ],
                    [
                        [
                            [1741756.244, 5420752.589],
                            [1741856.311, 5420752.589],
                            [1741856.311, 5420852.676],
                            [1741756.244, 5420852.676],
                            [1741756.244, 5420752.589]
                        ],
                        [
                            [1741771.259, 5420772.604],
                            [1741841.296, 5420772.604],
                            [1741841.296, 5420832.661],
                            [1741771.259, 5420832.661],
                            [1741771.259, 5420772.604]
                        ],
                        [
                            [1741786.274, 5420787.619],
                            [1741831.291, 5420787.619],
                            [1741831.291, 5420817.648],
                            [1741786.274, 5420817.648],
                            [1741786.274, 5420787.619]
                        ]
                    ]
                ],
                "shapeType": "MultiPolygon",
                "type": "MultiPolygon"
            },
            "type": "Feature"
        }
    ],
    "type": "FeatureCollection"
})";
static constexpr char kWktPoint[] = "POINT (1568455.878400 5179521.088200)";
static constexpr char kWktLine[] =
    ("LINESTRING (1570453.177587 5182792.085639, 1570463.685564 "
     "5182793.860214, 1570471.708698 5182793.679381, 1570479.612996 "
     "5182795.874611, 1570483.571308 5182795.567898, 1570490.786123 "
     "5182794.303225, 1570492.245839 5182795.173844, 1570493.481633 "
     "5182797.663887)");
static constexpr char kWktPolygon0[] =
    ("POLYGON ((1568287.246207 5179502.694713, 1568291.926131 5179501.564891, "
     "1568299.977034 5179500.490728, 1568305.678037 5179500.717903, "
     "1568356.622853 5179506.519054, 1568356.781250 5179505.128043, "
     "1568305.785251 5179499.321064, 1568299.911847 5179499.087020, "
     "1568291.668584 5179500.186847, 1568286.917660 5179501.333810, "
     "1568287.246207 5179502.694713))");
static constexpr char kWktPolygon1[] =
    ("POLYGON ((1740568.898000 5419812.500000, 1740578.998000 5419812.500000, "
     "1740578.998000 5419822.608000, 1740568.898000 5419822.608000, "
     "1740568.898000 5419812.500000), (1740571.950000 5419815.555000, "
     "1740575.945000 5419815.555000, 1740575.945000 5419819.552000, "
     "1740571.950000 5419819.552000, 1740571.950000 5419815.555000))");
static constexpr char kWktPolygon2[] =
    ("POLYGON ((1741756.244000 5420752.589000, 1741856.311000 5420752.589000, "
     "1741856.311000 5420852.676000, 1741756.244000 5420852.676000, "
     "1741756.244000 5420752.589000), (1741771.259000 5420772.604000, "
     "1741841.296000 5420772.604000, 1741841.296000 5420832.661000, "
     "1741771.259000 5420832.661000, 1741771.259000 5420772.604000), "
     "(1741786.274000 5420787.619000, 1741831.291000 5420787.619000, "
     "1741831.291000 5420817.648000, 1741786.274000 5420817.648000, "
     "1741786.274000 5420787.619000))");
static constexpr char kWktMpolygon[] =
    ("MULTIPOLYGON (((1572223.199000 5176019.291000, 1572223.200000 "
     "5176019.376000, 1572223.161000 5176019.331000, 1572223.199000 "
     "5176019.291000)), ((1741756.244000 5420752.589000, 1741856.311000 "
     "5420752.589000, 1741856.311000 5420852.676000, 1741756.244000 "
     "5420852.676000, 1741756.244000 5420752.589000), (1741771.259000 "
     "5420772.604000, 1741841.296000 5420772.604000, 1741841.296000 "
     "5420832.661000, 1741771.259000 5420832.661000, 1741771.259000 "
     "5420772.604000), (1741786.274000 5420787.619000, 1741831.291000 "
     "5420787.619000, 1741831.291000 5420817.648000, 1741786.274000 "
     "5420817.648000, 1741786.274000 5420787.619000)))");
static constexpr char kWktCsv[] =
    "id\troad\tside\tstart\tclose\tlength\twidth\tarea\tage\twkt\nfc6e0039-"
    "06ad-43f6-94cb-c703edfb63b8\tBLENHEIM RD1/ACCESS TO RLY "
    "LINE\tUnknown\t170\t0\t183\t0\t475.8\t15\tPOINT (1568455.878400 "
    "5179521.088200)\nfb9baee1-1d85-442d-b691-48fddf7ad861\tCOURTENAY "
    "ST\tRight\t190\t215\t42\t0\t63\t44\tLINESTRING (1570453.177587 "
    "5182792.085639, 1570463.685564 5182793.860214, 1570471.708698 "
    "5182793.679381, 1570479.612996 5182795.874611, 1570483.571308 "
    "5182795.567898, 1570490.786123 5182794.303225, 1570492.245839 "
    "5182795.173844, 1570493.481633 "
    "5182797.663887)\n7c0db807-1d97-4a2a-880a-d6faa32ecfac\tVenilla "
    "Polygon\tRight\t4\t67\t63\t0\t88.2\t17\tPOLYGON ((1568287.246207 "
    "5179502.694713, 1568291.926131 5179501.564891, 1568299.977034 "
    "5179500.490728, 1568305.678037 5179500.717903, 1568356.622853 "
    "5179506.519054, 1568356.781250 5179505.128043, 1568305.785251 "
    "5179499.321064, 1568299.911847 5179499.087020, 1568291.668584 "
    "5179500.186847, 1568286.917660 5179501.333810, 1568287.246207 "
    "5179502.694713))\n33f00889-b86e-4300-a069-428f1ae78531\tPolygon with "
    "One Hole\tLeft\t1\t173\t170\t0\t680\t2\tPOLYGON ((1740568.898000 "
    "5419812.500000, 1740578.998000 5419812.500000, 1740578.998000 "
    "5419822.608000, 1740568.898000 5419822.608000, 1740568.898000 "
    "5419812.500000), (1740571.950000 5419815.555000, 1740575.945000 "
    "5419815.555000, 1740575.945000 5419819.552000, 1740571.950000 "
    "5419819.552000, 1740571.950000 "
    "5419815.555000))\n36b1450b-f532-4a9b-9f6d-de52130cf741\tPolygon with "
    "Multiple Holes\tRight\t769\t777\t8\t0\t18.4\t24\tPOLYGON "
    "((1741756.244000 5420752.589000, 1741856.311000 5420752.589000, "
    "1741856.311000 5420852.676000, 1741756.244000 5420852.676000, "
    "1741756.244000 5420752.589000), (1741771.259000 5420772.604000, "
    "1741841.296000 5420772.604000, 1741841.296000 5420832.661000, "
    "1741771.259000 5420832.661000, 1741771.259000 5420772.604000), "
    "(1741786.274000 5420787.619000, 1741831.291000 5420787.619000, "
    "1741831.291000 5420817.648000, 1741786.274000 5420817.648000, "
    "1741786.274000 "
    "5420787.619000))\n9d9a8926-958e-472c-b4a5-861912d6709d\tAOTEA "
    "TCE\tRight\t475\t729\t254\t0\t381\t44\tMULTIPOLYGON (((1572223.199000 "
    "5176019.291000, 1572223.200000 5176019.376000, 1572223.161000 "
    "5176019.331000, 1572223.199000 5176019.291000)), ((1741756.244000 "
    "5420752.589000, 1741856.311000 5420752.589000, 1741856.311000 "
    "5420852.676000, 1741756.244000 5420852.676000, 1741756.244000 "
    "5420752.589000), (1741771.259000 5420772.604000, 1741841.296000 "
    "5420772.604000, 1741841.296000 5420832.661000, 1741771.259000 "
    "5420832.661000, 1741771.259000 5420772.604000), (1741786.274000 "
    "5420787.619000, 1741831.291000 5420787.619000, 1741831.291000 "
    "5420817.648000, 1741786.274000 5420817.648000, 1741786.274000 "
    "5420787.619000)))\n";

using namespace fdt::gis;

TEST(WKT, ToWktObject) {

    static const nlohmann::json &js = nlohmann::json::parse(kGeoJson);

    static const nlohmann::json &js_point = js["features"][0];
    static const nlohmann::json &js_line = js["features"][1];
    static const nlohmann::json &js_polygon0 = js["features"][2];
    static const nlohmann::json &js_polygon1 = js["features"][3];
    static const nlohmann::json &js_polygon2 = js["features"][4];
    static const nlohmann::json &js_mpolygon = js["features"][5];

    const std::string &wkt_point = json2wkt(js_point);
    const std::string &wkt_line = json2wkt(js_line);
    const std::string &wkt_polygon0 = json2wkt(js_polygon0);
    const std::string &wkt_polygon1 = json2wkt(js_polygon1);
    const std::string &wkt_polygon2 = json2wkt(js_polygon2);
    const std::string &wkt_mpolygon = json2wkt(js_mpolygon);

    EXPECT_STREQ(wkt_point.c_str(), kWktPoint);
    EXPECT_STREQ(wkt_line.c_str(), kWktLine);
    EXPECT_STREQ(wkt_polygon0.c_str(), kWktPolygon0);
    EXPECT_STREQ(wkt_polygon1.c_str(), kWktPolygon1);
    EXPECT_STREQ(wkt_polygon2.c_str(), kWktPolygon2);
    EXPECT_STREQ(wkt_mpolygon.c_str(), kWktMpolygon);
}

TEST(WKT, Json2Csv) {
    std::istringstream istream_str(kGeoJson);
    std::ostringstream ostream_str;
    Geojson2Tsv(istream_str, ostream_str);
    EXPECT_STREQ(ostream_str.str().c_str(), kWktCsv);
}

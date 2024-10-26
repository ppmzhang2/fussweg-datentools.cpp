#include "via.hpp"
#include <gtest/gtest.h>

using namespace fdt::via;

// Test enum class Fault
TEST(Fault, AndOrOperator) {
    Fault fault =
        Fault::CRACK_FAIR | Fault::BUMP_POOR | Fault::DEPRESSION_VPOOR;
    EXPECT_EQ(static_cast<int>(fault), 0b0000000000110110);
    fault |= (Fault::CRACK_VPOOR | Fault::UNEVEN_POOR);
    EXPECT_EQ(static_cast<int>(fault), 0b0000100000111110);
}

TEST(ImgBox, ParseFromCsv) {
    static const std::string str_csv =
        ("filename,file_size,file_attributes,region_count,region_id,region_"
         "shape_attributes,region_attributes\n"
         "G0017468.JPG,3142365,\"{}\",1,0,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":2428,\"\"y\"\":3509,\"\"width\"\":251,\"\"height\"\":1257}"
         "\",\"{\"\"fault\"\":{\"\"vegetation\"\":true},\"\"condition\"\":{"
         "\"\"fair\"\":true}}\"\n"
         "G0017469.JPG,3116698,\"{}\",1,0,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":1642,\"\"y\"\":2295,\"\"width\"\":1556,\"\"height\"\":198}"
         "\",\"{\"\"fault\"\":{\"\"uneven\"\":true,\"\"vegetation\"\":true},"
         "\"\"condition\"\":{\"\"poor\"\":true}}\"\n"
         "G0017471.JPG,3116001,\"{}\",1,0,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":2380,\"\"y\"\":3509,\"\"width\"\":139,\"\"height\"\":1316}"
         "\",\"{\"\"fault\"\":{\"\"pothole\"\":true},\"\"condition\"\":{"
         "\"\"fair\"\":true}}\"\n"
         "G0017472.JPG,3137214,\"{}\",0,0,\"{}\",\"{}\"\n"
         "G0017473.JPG,3112070,\"{}\",1,0,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":2273,\"\"y\"\":1471,\"\"width\"\":337,\"\"height\"\":2872}"
         "\",\"{\"\"fault\"\":{\"\"depression\"\":true},\"\"condition\"\":{"
         "\"\"verypoor\"\":true}}\"\n"
         "G0017485.JPG,3128748,\"{}\",1,0,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":2589,\"\"y\"\":3744,\"\"width\"\":53,\"\"height\"\":1113}"
         "\",\"{\"\"fault\"\":{\"\"bump\"\":true},\"\"condition\"\":{"
         "\"\"fair\"\":true}}\"\n"
         "G0017487.JPG,3160842,\"{}\",0,0,\"{}\",\"{}\"\n"
         "G0017490.JPG,3142702,\"{}\",1,0,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":1781,\"\"y\"\":1883,\"\"width\"\":947,\"\"height\"\":107}"
         "\",\"{\"\"fault\"\":{\"\"displacement\"\":true,\"\"vegetation\"\":"
         "true},"
         "\"\"condition\"\":{\"\"fair\"\":true}}\"\n"
         "G0017491.JPG,3142581,\"{}\",2,0,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":257,\"\"y\"\":3824,\"\"width\"\":2337,\"\"height\"\":160}"
         "\",\"{\"\"fault\"\":{\"\"crack\"\":true,\"\"vegetation\"\":true},"
         "\"\"condition\"\":{\"\"fair\"\":true}}\"\n"
         "G0017491.JPG,3142581,\"{}\",2,1,\"{\"\"name\"\":\"\"rect\"\","
         "\"\"x\"\":2573,\"\"y\"\":3156,\"\"width\"\":118,\"\"height\"\":690}"
         "\",\"{\"\"fault\"\":{\"\"crack\"\":true,\"\"vegetation\"\":true},"
         "\"\"condition\"\":{\"\"fair\"\":true}}\"\n");

    FaultStats stats;

    std::istringstream istream_csv(str_csv);
    ImgBoxArr ibx_arr = parseCsv(istream_csv);
    for (auto &ibx : ibx_arr) {
        for (auto &bx : ibx.boxes) {
            stats.AddFault(bx.fault);
        }
    }
    EXPECT_EQ(stats.bump_fair, 1);
    EXPECT_EQ(stats.bump_poor, 0);
    EXPECT_EQ(stats.bump_verypoor, 0);
    EXPECT_EQ(stats.crack_fair, 2);
    EXPECT_EQ(stats.crack_poor, 0);
    EXPECT_EQ(stats.crack_verypoor, 0);
    EXPECT_EQ(stats.depression_fair, 0);
    EXPECT_EQ(stats.depression_poor, 0);
    EXPECT_EQ(stats.depression_verypoor, 1);
    EXPECT_EQ(stats.displacement_fair, 1);
    EXPECT_EQ(stats.displacement_poor, 0);
    EXPECT_EQ(stats.displacement_verypoor, 0);
    EXPECT_EQ(stats.vegetation_fair, 4);
    EXPECT_EQ(stats.vegetation_poor, 1);
    EXPECT_EQ(stats.vegetation_verypoor, 0);
    EXPECT_EQ(stats.uneven_fair, 0);
    EXPECT_EQ(stats.uneven_poor, 1);
    EXPECT_EQ(stats.uneven_verypoor, 0);
    EXPECT_EQ(stats.pothole_fair, 1);
    EXPECT_EQ(stats.pothole_poor, 0);
    EXPECT_EQ(stats.pothole_verypoor, 0);
}

TEST(ImgBox, ParseFromJson) {
    static const std::string str_json =
        ("{\n"
         "  \"_via_settings\": {\n"
         "    \"ui\": {\n"
         "      \"annotation_editor_height\": 25,\n"
         "      \"annotation_editor_fontsize\": 0.8,\n"
         "      \"leftsidebar_width\": 18,\n"
         "      \"image_grid\": {\n"
         "        \"img_height\": 80,\n"
         "        \"rshape_fill\": \"none\",\n"
         "        \"rshape_fill_opacity\": 0.3,\n"
         "        \"rshape_stroke\": \"yellow\",\n"
         "        \"rshape_stroke_width\": 2,\n"
         "        \"show_region_shape\": true,\n"
         "        \"show_image_policy\": \"all\"\n"
         "      },\n"
         "      \"image\": {\n"
         "        \"region_label\": \"__via_region_id__\",\n"
         "        \"region_color\": \"condition\",\n"
         "        \"region_label_font\": \"10px Sans\",\n"
         "        \"on_image_annotation_editor_placement\": \"NEAR_REGION\"\n"
         "      }\n"
         "    },\n"
         "    \"core\": {\n"
         "      \"buffer_size\": \"18\",\n"
         "      \"filepath\": {},\n"
         "      \"default_filepath\": \"SomePath\"\n"
         "    },\n"
         "    \"project\": { \"name\": \"via_project_test\" }\n"
         "  },\n"
         "  \"_via_img_metadata\": {\n"
         "    \"G0060985.JPG3147783\": {\n"
         "      \"filename\": \"G0060985.JPG\",\n"
         "      \"size\": 3147783,\n"
         "      \"regions\": [\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 1540,\n"
         "            \"y\": 1240,\n"
         "            \"width\": 1013,\n"
         "            \"height\": 1127\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"vegetation\": true, \"displacement\": "
         "true },\n"
         "            \"condition\": { \"poor\": true }\n"
         "          }\n"
         "        },\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 2375,\n"
         "            \"y\": 470,\n"
         "            \"width\": 981,\n"
         "            \"height\": 559\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"vegetation\": true, \"displacement\": "
         "true },\n"
         "            \"condition\": { \"verypoor\": true }\n"
         "          }\n"
         "        }\n"
         "      ],\n"
         "      \"file_attributes\": {}\n"
         "    },\n"
         "    \"G0060986.JPG3098397\": {\n"
         "      \"filename\": \"G0060986.JPG\",\n"
         "      \"size\": 3098397,\n"
         "      \"regions\": [\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 592,\n"
         "            \"y\": 2521,\n"
         "            \"width\": 2156,\n"
         "            \"height\": 1864\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"bump\": true, \"depression\": true },\n"
         "            \"condition\": { \"fair\": true }\n"
         "          }\n"
         "        },\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 2561,\n"
         "            \"y\": 746,\n"
         "            \"width\": 1410,\n"
         "            \"height\": 932\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"uneven\": true, \"displacement\": true "
         "},\n"
         "            \"condition\": { \"poor\": true }\n"
         "          }\n"
         "        }\n"
         "      ],\n"
         "      \"file_attributes\": {}\n"
         "    },\n"
         "    \"G0060987.JPG3156212\": {\n"
         "      \"filename\": \"G0060987.JPG\",\n"
         "      \"size\": 3156212,\n"
         "      \"regions\": [\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 1905,\n"
         "            \"y\": 1013,\n"
         "            \"width\": 1953,\n"
         "            \"height\": 2277\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"bump\": true, \"crack\": true },\n"
         "            \"condition\": { \"verypoor\": true }\n"
         "          }\n"
         "        }\n"
         "      ],\n"
         "      \"file_attributes\": {}\n"
         "    },\n"
         "    \"G0060988.JPG3125030\": {\n"
         "      \"filename\": \"G0060988.JPG\",\n"
         "      \"size\": 3125030,\n"
         "      \"regions\": [\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 940,\n"
         "            \"y\": 1556,\n"
         "            \"width\": 3039,\n"
         "            \"height\": 3128\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"pothole\": true },\n"
         "            \"condition\": { \"verypoor\": true }\n"
         "          }\n"
         "        }\n"
         "      ],\n"
         "      \"file_attributes\": {}\n"
         "    },\n"
         "    \"G0060989.JPG3161032\": {\n"
         "      \"filename\": \"G0060989.JPG\",\n"
         "      \"size\": 3161032,\n"
         "      \"regions\": [\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 1062,\n"
         "            \"y\": 2829,\n"
         "            \"width\": 2691,\n"
         "            \"height\": 1986\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"bump\": true, \"uneven\": true },\n"
         "            \"condition\": { \"poor\": true }\n"
         "          }\n"
         "        }\n"
         "      ],\n"
         "      \"file_attributes\": {}\n"
         "    },\n"
         "    \"G0060990.JPG3181228\": {\n"
         "      \"filename\": \"G0060990.JPG\",\n"
         "      \"size\": 3181228,\n"
         "      \"regions\": [\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 940,\n"
         "            \"y\": 3850,\n"
         "            \"width\": 746,\n"
         "            \"height\": 916\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"pothole\": true },\n"
         "            \"condition\": { \"fair\": true }\n"
         "          }\n"
         "        }\n"
         "      ],\n"
         "      \"file_attributes\": {}\n"
         "    },\n"
         "    \"G0060993.JPG3175975\": {\n"
         "      \"filename\": \"G0060993.JPG\",\n"
         "      \"size\": 3175975,\n"
         "      \"regions\": [\n"
         "        {\n"
         "          \"shape_attributes\": {\n"
         "            \"name\": \"rect\",\n"
         "            \"x\": 559,\n"
         "            \"y\": 2877,\n"
         "            \"width\": 1807,\n"
         "            \"height\": 1718\n"
         "          },\n"
         "          \"region_attributes\": {\n"
         "            \"fault\": { \"crack\": true, \"depression\": true },\n"
         "            \"condition\": { \"poor\": true }\n"
         "          }\n"
         "        }\n"
         "      ],\n"
         "      \"file_attributes\": {}\n"
         "    }\n"
         "  },\n"
         "  \"_via_attributes\": {\n"
         "    \"region\": {\n"
         "      \"fault\": {\n"
         "        \"type\": \"checkbox\",\n"
         "        \"description\": \"\",\n"
         "        \"options\": {\n"
         "          \"bump\": \"\",\n"
         "          \"crack\": \"\",\n"
         "          \"depression\": \"\",\n"
         "          \"displacement\": \"\",\n"
         "          \"vegetation\": \"\",\n"
         "          \"uneven\": \"\",\n"
         "          \"pothole\": \"\"\n"
         "        },\n"
         "        \"default_options\": {}\n"
         "      },\n"
         "      \"condition\": {\n"
         "        \"type\": \"checkbox\",\n"
         "        \"description\": \"\",\n"
         "        \"options\": { \"fair\": \"\", \"poor\": \"\", \"verypoor\": "
         "\"\" },\n"
         "        \"default_options\": {}\n"
         "      }\n"
         "    },\n"
         "    \"file\": {}\n"
         "  },\n"
         "  \"_via_data_format_version\": \"2.0.10\",\n"
         "  \"_via_image_id_list\": [\n"
         "    \"G0060985.JPG3147783\",\n"
         "    \"G0060986.JPG3098397\",\n"
         "    \"G0060987.JPG3156212\",\n"
         "    \"G0060988.JPG3125030\",\n"
         "    \"G0060989.JPG3161032\",\n"
         "    \"G0060990.JPG3181228\",\n"
         "    \"G0060993.JPG3175975\"\n"
         "  ]\n"
         "}\n");

    FaultStats stats;

    std::istringstream istream_json(str_json);
    ImgBoxArr ibx_arr = parseJson(istream_json);
    for (auto &ibx : ibx_arr) {
        for (auto &bx : ibx.boxes) {
            stats.AddFault(bx.fault);
        }
    }

    EXPECT_EQ(stats.bump_fair, 1);
    EXPECT_EQ(stats.bump_poor, 1);
    EXPECT_EQ(stats.bump_verypoor, 1);
    EXPECT_EQ(stats.crack_fair, 0);
    EXPECT_EQ(stats.crack_poor, 1);
    EXPECT_EQ(stats.crack_verypoor, 1);
    EXPECT_EQ(stats.depression_fair, 1);
    EXPECT_EQ(stats.depression_poor, 1);
    EXPECT_EQ(stats.depression_verypoor, 0);
    EXPECT_EQ(stats.displacement_fair, 0);
    EXPECT_EQ(stats.displacement_poor, 2);
    EXPECT_EQ(stats.displacement_verypoor, 1);
    EXPECT_EQ(stats.vegetation_fair, 0);
    EXPECT_EQ(stats.vegetation_poor, 1);
    EXPECT_EQ(stats.vegetation_verypoor, 1);
    EXPECT_EQ(stats.uneven_fair, 0);
    EXPECT_EQ(stats.uneven_poor, 2);
    EXPECT_EQ(stats.uneven_verypoor, 0);
    EXPECT_EQ(stats.pothole_fair, 1);
    EXPECT_EQ(stats.pothole_poor, 0);
    EXPECT_EQ(stats.pothole_verypoor, 1);
}

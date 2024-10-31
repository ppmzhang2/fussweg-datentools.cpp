#include "ibox.hpp"
#include <gtest/gtest.h>

using namespace fdt::ibox;

// Test enum class Fault
TEST(Fault, AndOrOp) {
    Fault fault =
        Fault::CRACK_FAIR | Fault::BUMP_POOR | Fault::DEPRESSION_VPOOR;
    EXPECT_EQ(static_cast<int>(fault), 0b0000000000110110);
    fault |= (Fault::CRACK_VPOOR | Fault::VEGETATION_POOR);
    EXPECT_EQ(static_cast<int>(fault), 0b0010000000111110);
}

TEST(Box, MaxSeverityOp) {
    Box bx;
    bx.fault = Fault::NONE;
    EXPECT_EQ(bx.MaxSeverity(), static_cast<uint8_t>(0));
    bx.fault |= Fault::CRACK_FAIR;
    EXPECT_EQ(bx.MaxSeverity(), static_cast<uint8_t>(1));
    bx.fault |= Fault::BUMP_POOR;
    EXPECT_EQ(bx.MaxSeverity(), static_cast<uint8_t>(2));
    bx.fault |= Fault::DEPRESSION_VPOOR;
    EXPECT_EQ(bx.MaxSeverity(), static_cast<uint8_t>(3));
}

TEST(ImgBox, FromViaCsv) {
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

    std::istringstream istream_csv(str_csv);
    std::vector<ImgBox> ibx_arr = from_via_csv(istream_csv);

    std::array<uint8_t, 21> arr = {0};
    uint8_t idx_lvl;

    for (auto &ibx : ibx_arr) {
        for (auto &bx : ibx.boxes) {
            for (uint8_t idx_type = 0; idx_type < 7; ++idx_type) {
                idx_lvl =
                    (static_cast<uint16_t>(bx.fault) >> (idx_type * 2)) & 0b11;
                if (idx_lvl != 0) {
                    int idx = idx_type * 3 + (idx_lvl - 1);
                    arr[idx] += 1;
                }
            }
        }
    }

    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 0);
    EXPECT_EQ(arr[2], 0);
    EXPECT_EQ(arr[3], 2);
    EXPECT_EQ(arr[4], 0);
    EXPECT_EQ(arr[5], 0);
    EXPECT_EQ(arr[6], 0);
    EXPECT_EQ(arr[7], 0);
    EXPECT_EQ(arr[8], 1);
    EXPECT_EQ(arr[9], 1);
    EXPECT_EQ(arr[10], 0);
    EXPECT_EQ(arr[11], 0);
    EXPECT_EQ(arr[12], 1);
    EXPECT_EQ(arr[13], 0);
    EXPECT_EQ(arr[14], 0);
    EXPECT_EQ(arr[15], 0);
    EXPECT_EQ(arr[16], 1);
    EXPECT_EQ(arr[17], 0);
    EXPECT_EQ(arr[18], 4);
    EXPECT_EQ(arr[19], 1);
    EXPECT_EQ(arr[20], 0);
}

TEST(ImgBox, FromViaJson) {
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

    std::istringstream istream_json(str_json);
    std::vector<ImgBox> ibx_arr = from_via_json(istream_json);

    std::array<uint8_t, 21> arr = {0};
    uint8_t idx_lvl;

    for (auto &ibx : ibx_arr) {
        for (auto &bx : ibx.boxes) {
            for (uint8_t idx_type = 0; idx_type < 7; ++idx_type) {
                idx_lvl =
                    (static_cast<uint16_t>(bx.fault) >> (idx_type * 2)) & 0b11;
                if (idx_lvl != 0) {
                    int idx = idx_type * 3 + (idx_lvl - 1);
                    arr[idx] += 1;
                }
            }
        }
    }

    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 1);
    EXPECT_EQ(arr[2], 1);
    EXPECT_EQ(arr[3], 0);
    EXPECT_EQ(arr[4], 1);
    EXPECT_EQ(arr[5], 1);
    EXPECT_EQ(arr[6], 1);
    EXPECT_EQ(arr[7], 1);
    EXPECT_EQ(arr[8], 0);
    EXPECT_EQ(arr[9], 0);
    EXPECT_EQ(arr[10], 2);
    EXPECT_EQ(arr[11], 1);
    EXPECT_EQ(arr[12], 1);
    EXPECT_EQ(arr[13], 0);
    EXPECT_EQ(arr[14], 1);
    EXPECT_EQ(arr[15], 0);
    EXPECT_EQ(arr[16], 2);
    EXPECT_EQ(arr[17], 0);
    EXPECT_EQ(arr[18], 0);
    EXPECT_EQ(arr[19], 1);
    EXPECT_EQ(arr[20], 1);
}

TEST(ImgBox, FromTsv) {
    static const std::string str_tsv =
        ("prefix\timage\tx\ty\tw\th\tcategory\tscore_cate_top1\tclass_"
         "cate_top2\tscore_cate_top2\tlevel\tscore_lvl_top1\tclass_"
         "lvl_top2\tscore_lvl_top2\n"
         "20231115\tG0018488."
         "JPG\t3203\t563\t1166\t3925\tdepression\t92\tvegetation\t3\tpoor\t80\t"
         "verypoor"
         "\t19\n"
         "20231004\tG0033549."
         "JPG\t379\t46\t2090\t4148\tpothole\t68\tdepression\t22\tfair\t92\tp"
         "oor\t8\n"
         "20230920\tG0047026."
         "JPG\t1832\t1071\t1528\t194\tcrack\t71\tuneven\t22\tfair\t93\tpoor\t7"
         "\n"
         "20231010\tG0025306."
         "JPG\t3800\t476\t207\t165\tbump\t74\tpothole\t12\tfair\t94\tpoor\t6\n"
         "20231115\tG0017910."
         "JPG\t1149\t1697\t3153\t1002\tpothole\t89\tcrack\t7\tverypoor\t77\tpoo"
         "r"
         "\t21\n"
         "20230920\tG0053839."
         "JPG\t129\t1158\t2839\t2501\tcrack\t82\tvegetation\t7\tfair\t92\tpoor"
         "\t8\n"
         "20231010\tG0040312."
         "JPG\t1320\t1569\t1187\t2263\tcrack\t50\tuneven\t21\tpoor\t89\tfair\t1"
         "1\n"
         "20231010\tG0021377."
         "JPG\t2522\t523\t929\t4055\tvegetation\t57\tbump\t16\tfair\t93\tpoor\t"
         "7\n"
         "20231115\tG0018488."
         "JPG\t21\t3128\t4615\t473\tcrack\t93\tuneven\t3\tverypoor\t90\tpoor\t1"
         "0\n"
         "20231115\tG0018488."
         "JPG\t2255\t196\t2117\t4411\tvegetation\t79\tcrack\t14\tfair\t89\tpoor"
         "\t11\n");

    csv::CSVFormat format;
    format.delimiter('\t').header_row(0);
    std::istringstream istream_tsv(str_tsv);
    csv::CSVReader reader(istream_tsv, format);

    std::vector<ImgBox> ibx_arr = from_csv_reader(reader);

    std::array<uint8_t, 21> arr = {0};
    uint8_t idx_lvl;

    for (auto &ibx : ibx_arr) {
        for (auto &bx : ibx.boxes) {
            for (uint8_t idx_type = 0; idx_type < 7; ++idx_type) {
                idx_lvl =
                    (static_cast<uint16_t>(bx.fault) >> (idx_type * 2)) & 0b11;
                if (idx_lvl != 0) {
                    int idx = idx_type * 3 + (idx_lvl - 1);
                    arr[idx] += 1;
                }
            }
        }
    }

    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 0);
    EXPECT_EQ(arr[2], 0);
    EXPECT_EQ(arr[3], 2);
    EXPECT_EQ(arr[4], 1);
    EXPECT_EQ(arr[5], 1);
    EXPECT_EQ(arr[6], 0);
    EXPECT_EQ(arr[7], 1);
    EXPECT_EQ(arr[8], 0);
    EXPECT_EQ(arr[9], 0);
    EXPECT_EQ(arr[10], 0);
    EXPECT_EQ(arr[11], 0);
    EXPECT_EQ(arr[12], 1);
    EXPECT_EQ(arr[13], 0);
    EXPECT_EQ(arr[14], 1);
    EXPECT_EQ(arr[15], 0);
    EXPECT_EQ(arr[16], 0);
    EXPECT_EQ(arr[17], 0);
    EXPECT_EQ(arr[18], 2);
    EXPECT_EQ(arr[19], 0);
    EXPECT_EQ(arr[20], 0);
}

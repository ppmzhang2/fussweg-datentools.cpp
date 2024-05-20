#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>

namespace fdt {
    namespace cv {

        struct Quad {

            ::cv::Point2d *tl;
            ::cv::Point2d *tr;
            ::cv::Point2d *br;
            ::cv::Point2d *bl;

            void print() const {
                std::cout << "TL -> " << tl->x << ", " << tl->y << std::endl;
                std::cout << "TR -> " << tr->x << ", " << tr->y << std::endl;
                std::cout << "BR -> " << br->x << ", " << br->y << std::endl;
                std::cout << "BL -> " << bl->x << ", " << bl->y << std::endl;
            }

            std::vector<::cv::Point2f> vector() const {
                std::vector<::cv::Point2f> vec;
                vec.push_back(*this->tl);
                vec.push_back(*this->tr);
                vec.push_back(*this->br);
                vec.push_back(*this->bl);
                return vec;
            }
        };

        nlohmann::json getOfDiffJson(const std::string &);

        void getQuadRoi(const int &, const int &, const Quad &, const Quad &,
                        Quad &);

        void transPerspe(const Quad &, const double &, const double &,
                         const std::string &, const std::string &);

    } // namespace cv
} // namespace fdt

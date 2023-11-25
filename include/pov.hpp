#include <iostream>
#include <opencv2/opencv.hpp>

typedef struct Quad {

    cv::Point2d *tl;
    cv::Point2d *tr;
    cv::Point2d *br;
    cv::Point2d *bl;

    void print() const {
        std::cout << "TL -> " << tl->x << ", " << tl->y << std::endl;
        std::cout << "TR -> " << tr->x << ", " << tr->y << std::endl;
        std::cout << "BR -> " << br->x << ", " << br->y << std::endl;
        std::cout << "BL -> " << bl->x << ", " << bl->y << std::endl;
    }

    std::vector<cv::Point2f> vector() const {
        std::vector<cv::Point2f> vec;
        vec.push_back(*this->tl);
        vec.push_back(*this->tr);
        vec.push_back(*this->br);
        vec.push_back(*this->bl);
        return vec;
    }

} Quad;

namespace PoV {

    static inline void intersect(const cv::Point2d &init1,
                                 const cv::Point2d &term1,
                                 const cv::Point2d &init2,
                                 const cv::Point2d &term2,
                                 cv::Point2d &common) {
        // Calculating slopes (m1 and m2) and intercepts (c1 and c2)
        const double k1 = (term1.y - init1.y) / (term1.x - init1.x);
        const double k2 = (term2.y - init2.y) / (term2.x - init2.x);

        // Calculating intercepts
        const double b1 = init1.y - k1 * init1.x;
        const double b2 = init2.y - k2 * init2.x;

        // Finding intersection point
        common.x = (b2 - b1) / (k1 - k2);
        common.y = k1 * common.x + b1;
    }

    // Get perspective ROI based on vanishing points
    static inline void roi_from_vp(const Quad &img, const cv::Point2d &vp_top,
                                   const cv::Point2d &vp_side, Quad &roi) {
        if (vp_side.x < 0 && vp_side.y > img.br->y) {
            // left vanishing, lower than bottom
            intersect(*img.bl, vp_top, *img.tr, vp_side, *roi.tl);
            intersect(*img.br, vp_top, *img.tr, vp_side, *roi.tr);
            intersect(*img.br, vp_top, *img.bl, vp_side, *roi.br);
            *roi.bl = *img.bl;
        } else if (vp_side.x < 0 && vp_side.y < 0) {
            // left vanishing, higher than top
            intersect(*img.bl, vp_top, *img.tl, vp_side, *roi.tl);
            intersect(*img.br, vp_top, *img.tl, vp_side, *roi.tr);
            *roi.br = *img.br;
            intersect(*img.bl, vp_top, *img.br, vp_side, *roi.bl);
        } else if (vp_side.x < 0) {
            // left vanishing, between top and bottom
            intersect(*img.bl, vp_top, *img.tr, vp_side, *roi.tl);
            intersect(*img.br, vp_top, *img.tr, vp_side, *roi.tr);
            *roi.br = *roi.br;
            intersect(*img.bl, vp_top, *img.br, vp_side, *roi.bl);
        } else if (vp_side.x > 0 && vp_side.y > img.br->y) {
            // right vanishing, lower than bottom
            intersect(*img.bl, vp_top, *img.tl, vp_side, *roi.tl);
            intersect(*img.br, vp_top, *img.tl, vp_side, *roi.tr);
            *roi.br = *img.br;
            intersect(*img.bl, vp_top, *img.br, vp_side, *roi.br);
        } else if (vp_side.x > 0 && vp_side.y < 0) {
            // right vanishing, higher than top
            intersect(*img.bl, vp_top, *img.tr, vp_side, *roi.tl);
            intersect(*img.br, vp_top, *img.tr, vp_side, *roi.tr);
            intersect(*img.br, vp_top, *img.bl, vp_side, *roi.br);
            *roi.bl = *img.bl;
        } else if (vp_side.x > 0) {
            // right vanishing, between top and bottom
            intersect(*img.bl, vp_top, *img.tl, vp_side, *roi.tl);
            intersect(*img.br, vp_top, *img.tl, vp_side, *roi.tr);
            intersect(*img.br, vp_top, *img.bl, vp_side, *roi.br);
            *roi.bl = *img.bl;
        } else {
            // raise error
            std::cout << "Error: No vanishing point found" << std::endl;
        }
    }

    void get_roi(const int &, const int &, const Quad &, const Quad &, Quad &);

    void transform(const Quad &, const double &, const double &,
                   const std::string &, const std::string &);

} // namespace PoV

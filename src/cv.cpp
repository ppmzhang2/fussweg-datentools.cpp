#include <algorithm>
#include <future>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "cv.hpp"
#include "utils.hpp"

// Computes the optical flow diff between two images and returns the mean
// absolute value of the flow.
//
// @param imgAPath Path to the first image.
// @param imgBPath Path to the second image.
// @return The mean absolute value of the optical flow.
static inline double get_of_diff(const std::string &path_img_a,
                                 const std::string &path_img_b) {
    const cv::Mat imgA = cv::imread(path_img_a, cv::IMREAD_GRAYSCALE);
    const cv::Mat imgB = cv::imread(path_img_b, cv::IMREAD_GRAYSCALE);

    cv::Mat flow;
    // prefer Farneback over Lucas-Kanade
    cv::calcOpticalFlowFarneback(imgA, imgB, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

    cv::Mat absFlow;
    cv::absdiff(flow, cv::Scalar::all(0), absFlow);

    return cv::mean(absFlow)[0];
}

// Computes the optical flow diff between a chunk of images.
// @param file_chunk A chunk of images.
// @return A json object containing the displacement between each image in the
// chunk.
static inline nlohmann::json get_of_diff_json_1p(const Paths &file_chunk) {
    nlohmann::json output;

    for (size_t i = 0; i < file_chunk.size() - 1; ++i) {
        nlohmann::json j;
        const double dis = get_of_diff(file_chunk[i], file_chunk[i + 1]);

        j["displacement"] = dis;
        j["file_a"] = file_chunk[i];
        j["file_b"] = file_chunk[i + 1];

        output.push_back(j);
    }
    return output;
}

// Computes the optical flow diff between all image pairs in a directory.
nlohmann::json fdt::cv::getOfDiffJson(const std::string &dir) {
    nlohmann::json out;
    Paths imgs = fdt::utils::listAllImages(dir);

    // Sort images by name
    std::sort(imgs.begin(), imgs.end());

    // Number of processes to be used
    size_t n_proc =
        std::min(imgs.size() / 2, (size_t)std::thread::hardware_concurrency());
    size_t csize = imgs.size() / n_proc; // chunk size

    std::vector<std::future<nlohmann::json>> futures;

    for (size_t i = 0; i < n_proc; ++i) {
        Paths chunk(i == 0 ? imgs.begin() : imgs.begin() + i * csize - 1,
                    i == n_proc - 1 ? imgs.end()
                                    : imgs.begin() + (i + 1) * csize);

        futures.push_back(
            std::async(std::launch::async, get_of_diff_json_1p, chunk));
    }

    for (auto &fut : futures) {
        nlohmann::json chunk_output = fut.get();
        for (const auto &item : chunk_output) {
            out.push_back(item);
        }
    }

    return out;
}

static inline void intersect(const cv::Point2d &init1, const cv::Point2d &term1,
                             const cv::Point2d &init2, const cv::Point2d &term2,
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
static inline void roi_from_vp(const fdt::cv::Quad &img,
                               const cv::Point2d &vp_top,
                               const cv::Point2d &vp_side, fdt::cv::Quad &roi) {
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

void fdt::cv::getQuadRoi(const int &height, const int &width,
                         const fdt::cv::Quad &vert, const fdt::cv::Quad &horz,
                         fdt::cv::Quad &roi) {
    ::cv::Point2d vp_top, vp_side;
    intersect(*vert.bl, *vert.tl, *vert.br, *vert.tr, vp_top);
    intersect(*horz.tl, *horz.tr, *horz.bl, *horz.br, vp_side);
    const Quad img = {
        .tl = new ::cv::Point2d{0, 0},
        .tr = new ::cv::Point2d{(double)width, 0},
        .br = new ::cv::Point2d{(double)width, (double)height},
        .bl = new ::cv::Point2d{0, (double)height},
    };
    roi_from_vp(img, vp_top, vp_side, roi);
}

static void trans_persp_one(const Paths &files, const cv::Mat &mat_homo,
                            const double &dst_width, const double &dst_height,
                            const std::string &dst_dir) {
    for (size_t i = 0; i < files.size(); ++i) {
        std::string img_file = files[i];
        cv::Mat img = cv::imread(img_file);

        // Apply the perspective transformation
        cv::Mat img_topdown;
        cv::warpPerspective(img, img_topdown, mat_homo,
                            cv::Size(dst_width, dst_height));

        // Save the transformed image to the destination folder
        std::string dst_path =
            std::filesystem::path(dst_dir)
                .append(std::filesystem::path(img_file).filename().string())
                .string();
        cv::imwrite(dst_path, img_topdown);
    }
}

void fdt::cv::transPerspe(const Quad &roi, const double &dst_width,
                          const double &dst_height, const std::string &src_dir,
                          const std::string &dst_dir) {

    // Destination points for the perspective transformation
    Quad dst = {
        .tl = new ::cv::Point2d{0, 0},
        .tr = new ::cv::Point2d{dst_width, 0},
        .br = new ::cv::Point2d{dst_width, dst_height},
        .bl = new ::cv::Point2d{0, dst_height},
    };

    // Compute the perspective transformation matrix
    ::cv::Mat mat_homo =
        ::cv::getPerspectiveTransform(roi.vector(), dst.vector());

    // Iterate over images in the source folder
    Paths imgs = fdt::utils::listAllImages(src_dir);

    // Number of processes to be used
    size_t n_proc =
        std::min(imgs.size() / 2, (size_t)std::thread::hardware_concurrency());
    size_t csize = imgs.size() / n_proc; // chunk size

    std::vector<std::future<void>> futures;

    auto task = [&mat_homo, &dst_width, &dst_height,
                 &dst_dir](const Paths imgs) {
        return trans_persp_one(imgs, mat_homo, dst_width, dst_height, dst_dir);
    };

    for (size_t i = 0; i < n_proc; ++i) {
        Paths chunk(i == 0 ? imgs.begin() : imgs.begin() + i * csize,
                    i == n_proc - 1 ? imgs.end()
                                    : imgs.begin() + (i + 1) * csize);

        futures.push_back(std::async(std::launch::async, task, chunk));
    }

    for (auto &fut : futures) {
        fut.get();
    }
}

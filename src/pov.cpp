#include <filesystem>
#include <future>

#include "path_finder.hpp"
#include "pov.hpp"

namespace fs = std::filesystem;

void PoV::get_roi(const int &height, const int &width, const Quad &vert,
                  const Quad &horz, Quad &roi) {
    cv::Point2d vp_top, vp_side;
    intersect(*vert.bl, *vert.tl, *vert.br, *vert.tr, vp_top);
    intersect(*horz.tl, *horz.tr, *horz.bl, *horz.br, vp_side);
    const Quad img = {
        .tl = new cv::Point2d{0, 0},
        .tr = new cv::Point2d{(double)width, 0},
        .br = new cv::Point2d{(double)width, (double)height},
        .bl = new cv::Point2d{0, (double)height},
    };
    roi_from_vp(img, vp_top, vp_side, roi);
}

static void transform_single(const Paths &files, const cv::Mat &mat_homo,
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
            fs::path(dst_dir)
                .append(fs::path(img_file).filename().string())
                .string();
        cv::imwrite(dst_path, img_topdown);
    }
}

void PoV::transform(const Quad &roi, const double &dst_width,
                    const double &dst_height, const std::string &src_dir,
                    const std::string &dst_dir) {

    // Destination points for the perspective transformation
    Quad dst = {
        .tl = new cv::Point2d{0, 0},
        .tr = new cv::Point2d{dst_width, 0},
        .br = new cv::Point2d{dst_width, dst_height},
        .bl = new cv::Point2d{0, dst_height},
    };

    // Compute the perspective transformation matrix
    cv::Mat mat_homo = cv::getPerspectiveTransform(roi.vector(), dst.vector());

    // Iterate over images in the source folder
    Paths imgs = PathFinder::AllFiles(src_dir);

    // Number of processes to be used
    size_t n_proc =
        std::min(imgs.size() / 2, (size_t)std::thread::hardware_concurrency());
    size_t csize = imgs.size() / n_proc; // chunk size

    std::vector<std::future<void>> futures;

    auto task = [&mat_homo, &dst_width, &dst_height,
                 &dst_dir](const Paths imgs) {
        return transform_single(imgs, mat_homo, dst_width, dst_height, dst_dir);
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

#include "optical_flow.hpp"
#include "path_finder.hpp"
#include <algorithm>
#include <future>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// Computes the optical flow between two images and returns the mean absolute
// value of the flow.
//
// @param imgAPath Path to the first image.
// @param imgBPath Path to the second image.
// @return The mean absolute value of the optical flow.
inline double reduce_mean_abs(const std::string &imgAPath,
                              const std::string &imgBPath) {
    const cv::Mat imgA = cv::imread(imgAPath, cv::IMREAD_GRAYSCALE);
    const cv::Mat imgB = cv::imread(imgBPath, cv::IMREAD_GRAYSCALE);

    cv::Mat flow;
    // prefer Farneback over Lucas-Kanade
    cv::calcOpticalFlowFarneback(imgA, imgB, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

    cv::Mat absFlow;
    cv::absdiff(flow, cv::Scalar::all(0), absFlow);

    return cv::mean(absFlow)[0];
}

// Computes the optical flow between a chunk of images.
// @param file_chunk A chunk of images.
// @return A json object containing the displacement between each image in the
// chunk.
nlohmann::json process_chunk(const Paths &file_chunk) {
    nlohmann::json output;

    for (size_t i = 0; i < file_chunk.size() - 1; ++i) {
        nlohmann::json j;
        const double dis = reduce_mean_abs(file_chunk[i], file_chunk[i + 1]);

        j["displacement"] = dis;
        j["file_a"] = file_chunk[i];
        j["file_b"] = file_chunk[i + 1];

        output.push_back(j);
    }
    return output;
}

nlohmann::json OpticalFlow::Displacement(const std::string &dir) {
    nlohmann::json out;
    Paths imgs = PathFinder::AllImages(dir);

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

        futures.push_back(std::async(std::launch::async, process_chunk, chunk));
    }

    for (auto &fut : futures) {
        nlohmann::json chunk_output = fut.get();
        for (const auto &item : chunk_output) {
            out.push_back(item);
        }
    }

    return out;
}

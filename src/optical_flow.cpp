#include <algorithm>
#include <future>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

typedef std::vector<std::string> Paths;

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

namespace OpticalFlow {

    nlohmann::json process_chunk(const Paths &img_files_chunk) {
        nlohmann::json output;

        for (size_t i = 0; i < img_files_chunk.size() - 1; ++i) {
            nlohmann::json j;
            const double dis =
                reduce_mean_abs(img_files_chunk[i], img_files_chunk[i + 1]);

            j["displacement"] = dis;
            j["file_a"] = img_files_chunk[i];
            j["file_b"] = img_files_chunk[i + 1];

            output.push_back(j);
        }
        return output;
    }

    nlohmann::json Displacement(const std::string &folder_path) {
        nlohmann::json final_output;
        Paths imgs;

        cv::glob(folder_path + "/*.JPG", imgs);
        std::sort(imgs.begin(), imgs.end());

        // Number of processes to be used
        size_t n_process = std::min(
            imgs.size() / 2, (size_t)std::thread::hardware_concurrency());
        size_t chunk_size = imgs.size() / n_process;

        std::vector<std::future<nlohmann::json>> futures;

        for (size_t i = 0; i < n_process; ++i) {
            Paths chunk(
                i == 0 ? imgs.begin() : imgs.begin() + i * chunk_size - 1,
                i == n_process - 1 ? imgs.end()
                                   : imgs.begin() + (i + 1) * chunk_size);

            futures.push_back(
                std::async(std::launch::async, process_chunk, chunk));
        }

        for (auto &fut : futures) {
            nlohmann::json chunk_output = fut.get();
            for (const auto &item : chunk_output) {
                final_output.push_back(item);
            }
        }

        return final_output;
    }
} // namespace OpticalFlow

#include "img.hpp"
#include "csv.hpp"
#include "utils.hpp"
#include <future>
#include <iostream>
#include <opencv2/opencv.hpp>

// Function to process each record and save the bounding box as an image
// Handles one TSV file at a time
void crop_bbox(const std::string &root_dir, const std::string &tsv_file,
               const std::string &output_dir, const int width,
               const int height) {
    // Create a TSV reader
    csv::CSVFormat format;
    format.delimiter('\t').header_row(0);
    csv::CSVReader reader(tsv_file, format);

    // Iterate over each row
    for (auto &row : reader) {
        const auto prefix = row["prefix"].get<std::string>();
        const auto image_name = row["image"].get<std::string>();
        const auto category = row["cate"].get<std::string>();
        const auto level = row["level"].get<std::string>();

        // NOTE: The bounding box coordinates can be DIRTY, check the raw data
        // first in case they are totally incorrect e.g. all negative values
        // TODO:
        // - why all -1 values?
        // - any other invalid cases?
        if (row["w"].get<int>() <= 0 || row["h"].get<int>() <= 0 ||
            row["x"].get<int>() >= width || row["y"].get<int>() >= height) {
            std::cerr << "Skipping invalid bounding box: prefix: " << prefix
                      << "; image: " << image_name << std::endl;
            continue;
        }
        // Assertion:
        // - x = max(0, x); y = max(0, y)
        // - w = min(w, width - x); h = min(h, height - y)
        const int x = MAX2(0, row["x"].get<int>());
        const int y = MAX2(0, row["y"].get<int>());
        const int w = MIN2(row["w"].get<int>(), width - x);
        const int h = MIN2(row["h"].get<int>(), height - y);

        // Load the image with std::filesystem by combining the root dir,
        // prefix, and image name
        std::string image_path =
            std::filesystem::path(root_dir) / prefix / image_name;

        cv::Mat image = cv::imread(image_path);
        // NOTE: cv::imread() silently returns an empty matrix if it fails to
        // load the image, rendering try-catch blocks useless
        if (image.empty()) {
            std::cerr << "Could not open or find the image: " << image_path
                      << std::endl;
            continue;
        }

        // final check
        try {
            // Extract the bounding box
            cv::Rect bounding_box(x, y, w, h);
            cv::Mat cropped_image = image(bounding_box);

            // Create the output image name
            std::string output_name =
                "_" + category + "_" + level + "_x" + std::to_string(x) + "_y" +
                std::to_string(y) + "_w" + std::to_string(w) + "_h" +
                std::to_string(h) + "_" + prefix + "_" + image_name;

            // Save the cropped image
            std::string output_path = output_dir + "/" + output_name;
            imwrite(output_path, cropped_image);
        } catch (const cv::Exception &e) {
            std::cerr << "OpenCV exception: " << e.what() << std::endl;
            std::cerr << "prefix: " << prefix << "; image: " << image_name
                      << std::endl;
            throw e;
        }
    }
}

// Crop the bounding box and save the image.
// TSV files in `tsv_dir` will all be read first and combined, before being
// shuffled and split to different threads, each of which will then process
// the images and save them to the output directory.
void fdt::img::bboxCrop(const std::string &root_dir, const std::string &tsv_dir,
                        const std::string &output_path, const int width,
                        const int height) {
    // Vector to hold future objects
    std::vector<std::future<void>> futures;

    // Launch a thread for each TSV file
    for (const auto &f : fdt::utils::listAllFiles(tsv_dir, ".tsv")) {
        futures.push_back(std::async(std::launch::async, crop_bbox, root_dir, f,
                                     output_path, width, height));
    }

    // Wait for all threads to finish
    for (auto &f : futures) {
        f.get();
    }
}

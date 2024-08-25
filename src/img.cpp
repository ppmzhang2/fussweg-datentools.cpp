#include "img.hpp"
#include "csv.hpp"
#include "utils.hpp"
#include <future>
#include <iostream>
#include <opencv2/opencv.hpp>

// Function to process each record and save the bounding box as an image
// Handles one TSV file at a time
void crop_bbox(const std::string &root_dir, const std::string &tsv_file,
               const std::string &output_dir) {
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
        const int x = row["x"].get<int>();
        const int y = row["y"].get<int>();
        const int w = row["w"].get<int>();
        const int h = row["h"].get<int>();

        // Load the image with std::filesystem by combining the root dir,
        // prefix, and image name
        std::string image_path =
            std::filesystem::path(root_dir) / prefix / image_name;

        cv::Mat image = cv::imread(image_path);

        if (image.empty()) {
            std::cerr << "Could not open or find the image: " << image_path
                      << std::endl;
            continue;
        }

        // Extract the bounding box
        cv::Rect bounding_box(x, y, w, h);
        cv::Mat cropped_image = image(bounding_box);

        // Create the output image name
        std::string output_name =
            "_" + category + "_" + level + "_" + prefix + "_" + image_name;

        // Save the cropped image
        std::string output_path = output_dir + "/" + output_name;
        imwrite(output_path, cropped_image);
    }
}

// Crop the bounding box and save the image.
// TSV files in `tsv_dir` will all be read first and combined, before being
// shuffled and split to different threads, each of which will then process
// the images and save them to the output directory.
void fdt::img::bboxCrop(const std::string &root_dir, const std::string &tsv_dir,
                        const std::string &output_path) {
    // Vector to hold future objects
    std::vector<std::future<void>> futures;

    // Launch a thread for each TSV file
    for (const auto &f : fdt::utils::listAllFiles(tsv_dir, ".tsv")) {
        futures.push_back(std::async(std::launch::async, crop_bbox, root_dir, f,
                                     output_path));
    }

    // Wait for all threads to finish
    for (auto &f : futures) {
        f.get();
    }
}

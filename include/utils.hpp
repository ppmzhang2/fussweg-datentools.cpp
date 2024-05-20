#pragma once

#include <fstream>
#include <string>
#include <vector>

#define MAX2(x, y) ((x) > (y) ? (x) : (y))

typedef std::vector<std::string> Paths;

namespace fdt {
    namespace utils {
        inline void writeFile(const std::string &out_file,
                              const std::string &content) {
            std::ofstream outFile(out_file);
            if (!outFile) {
                throw std::runtime_error("Unable to open for writing: " +
                                         out_file);
            }
            outFile << content << std::endl;
            outFile.close();
        }

        inline Paths listAllImages(const std::string &dir) {
            Paths imgs;
            for (const auto &f :
                 std::filesystem::recursive_directory_iterator(dir)) {
                if (f.path().extension() == ".JPG" ||
                    f.path().extension() == ".jpg") {
                    imgs.push_back(f.path().string());
                }
            }
            return imgs;
        }

        inline Paths listAllFiles(const std::string &dir,
                                  const std::string &ext) {
            Paths files;
            std::string ext_l = ext;
            std::string ext_u = ext;
            std::transform(ext_l.begin(), ext_l.end(), ext_l.begin(),
                           ::tolower);
            std::transform(ext_u.begin(), ext_u.end(), ext_u.begin(),
                           ::toupper);
            for (const auto &f :
                 std::filesystem::recursive_directory_iterator(dir)) {
                if (f.path().extension() == ext_l ||
                    f.path().extension() == ext_u) {
                    files.push_back(f.path().string());
                }
            }
            return files;
        }

    } // namespace utils

} // namespace fdt

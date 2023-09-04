#include "file_iter.hpp"

std::vector<std::string> jpegFiles;
std::vector<std::string>::iterator currentIter;

FileIter::FileIter(const std::string &dir) {
    for (const auto &f : std::filesystem::recursive_directory_iterator(dir)) {
        if (f.path().extension() == ".JPG" || f.path().extension() == ".jpg") {
            this->jpegFiles.push_back(f.path().string());
        }
    }
    this->currentIter = this->jpegFiles.begin();
}

std::vector<std::string>::iterator FileIter::begin() {
    return this->jpegFiles.begin();
}
std::vector<std::string>::iterator FileIter::end() {
    return this->jpegFiles.end();
}

bool FileIter::hasNext() { return this->currentIter != this->jpegFiles.end(); }

std::string FileIter::next() {
    if (this->hasNext()) {
        return *(this->currentIter++);
    }
    throw std::out_of_range("No more JPEG files.");
}

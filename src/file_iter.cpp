#include "file_iter.hpp"

JPEGFilesIterator::JPEGFilesIterator(const std::string &dir) {
    for (const auto &f : std::filesystem::recursive_directory_iterator(dir)) {
        if (f.path().extension() == ".JPG" || f.path().extension() == ".jpg") {
            jpegFiles.push_back(f.path().string());
        }
    }
    currentIter = jpegFiles.begin();
}

std::vector<std::string>::iterator JPEGFilesIterator::begin() {
    return jpegFiles.begin();
}

std::vector<std::string>::iterator JPEGFilesIterator::end() {
    return jpegFiles.end();
}

bool JPEGFilesIterator::hasNext() { return currentIter != jpegFiles.end(); }

std::string JPEGFilesIterator::next() {
    if (hasNext()) {
        return *(currentIter++);
    }
    throw std::out_of_range("No more JPEG files.");
}

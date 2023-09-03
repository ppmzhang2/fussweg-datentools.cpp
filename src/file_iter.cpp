#include "file_iter.hpp"

JPEGFilesIterator::JPEGFilesIterator(const std::string &directory) {
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".JPG" ||
            entry.path().extension() == ".jpg") {
            jpegFiles.push_back(entry.path().string());
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

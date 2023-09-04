#pragma once

#include <filesystem>
#include <vector>

class JPEGFilesIterator {
  public:
    JPEGFilesIterator(const std::string &directory);

    std::vector<std::string>::iterator begin();
    std::vector<std::string>::iterator end();

    bool hasNext();
    std::string next();

  private:
    std::vector<std::string> jpegFiles;
    std::vector<std::string>::iterator currentIter;
};

#pragma once
#include <string>

struct FileType {
    std::string name;
};

FileType detectFileType(const std::string& filename);

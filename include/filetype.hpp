#pragma once
#include <string>
#include <vector>

struct FileType {
    std::string name;
    std::vector<std::string> extensions;
    std::string singleLineComment;
    std::string mlCommentStart;
    std::string mlCommentEnd;
    std::vector<std::string> keywords1;
    std::vector<std::string> keywords2;
    bool highlightNumbers = false;
    bool highlightStrings = false;
};

FileType cppSyntax();
FileType detectFileType(const std::string& filename);

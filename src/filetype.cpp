#include "filetype.hpp"

FileType detectFileType(const std::string& filename) {
    if (filename.empty()) return {"Plain Text"};

    // Match whole filenames first
    auto lastSlash = filename.rfind('/');
    std::string base = (lastSlash != std::string::npos)
        ? filename.substr(lastSlash + 1) : filename;

    if (base == "Makefile" || base == "CMakeLists.txt")
        return {"Makefile"};

    // Match by extension
    auto dot = filename.rfind('.');
    if (dot == std::string::npos) return {"Plain Text"};

    std::string ext = filename.substr(dot);

    if (ext == ".c" || ext == ".h") return {"C"};
    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" ||
        ext == ".hpp" || ext == ".hxx") return {"C++"};
    if (ext == ".py") return {"Python"};
    if (ext == ".js") return {"JavaScript"};
    if (ext == ".rs") return {"Rust"};
    if (ext == ".java") return {"Java"};
    if (ext == ".sh" || ext == ".bash") return {"Shell"};
    if (ext == ".md") return {"Markdown"};
    if (ext == ".txt") return {"Text"};

    return {"Plain Text"};
}

#include "filetype.hpp"

FileType cppSyntax() {
    return {
        "C++",
        {".cpp", ".cc", ".cxx", ".hpp", ".hxx"},
        "//",
        "/*", "*/",
        {"if", "else", "while", "for", "do", "switch", "case", "default",
         "break", "continue", "return", "struct", "class", "enum", "union",
         "typedef", "static", "const", "extern", "inline", "volatile",
         "sizeof", "namespace", "using", "template", "typename",
         "public", "private", "protected", "virtual", "override",
         "try", "catch", "throw", "new", "delete", "nullptr",
         "#include", "#define", "#ifdef", "#ifndef", "#endif", "#pragma"},
        {"int", "long", "short", "double", "float", "char", "void",
         "unsigned", "signed", "bool", "auto", "size_t", "string",
         "vector", "true", "false"},
        true,
        true
    };
}

FileType detectFileType(const std::string& filename) {
    static const std::vector<FileType> syntaxDefs = {
        cppSyntax(),
        {"C", {".c", ".h"}, "//", "/*", "*/", {}, {}, true, true},
        {"Python", {".py"}, "#", "", "", {}, {}, false, false},
        {"JavaScript", {".js"}, "//", "/*", "*/", {}, {}, false, false},
        {"Rust", {".rs"}, "//", "/*", "*/", {}, {}, false, false},
        {"Java", {".java"}, "//", "/*", "*/", {}, {}, false, false},
        {"Shell", {".sh", ".bash"}, "#", "", "", {}, {}, false, false},
        {"Markdown", {".md"}, "", "", "", {}, {}, false, false},
        {"Text", {".txt"}, "", "", "", {}, {}, false, false},
        {"Makefile", {"Makefile", "CMakeLists.txt"}, "#", "", "", {}, {}, false, false},
    };

    if (filename.empty()) return {"Plain Text", {}, "", "", "", {}, {}, false, false};

    auto lastSlash = filename.rfind('/');
    std::string base = (lastSlash != std::string::npos)
        ? filename.substr(lastSlash + 1) : filename;

    auto dot = filename.rfind('.');
    std::string ext = (dot != std::string::npos) ? filename.substr(dot) : "";

    // Match whole filenames first (e.g., Makefile, CMakeLists.txt)
    for (const auto& ft : syntaxDefs) {
        for (const auto& pattern : ft.extensions) {
            if (pattern == base)
                return ft;
        }
    }

    // Then match by extension
    for (const auto& ft : syntaxDefs) {
        for (const auto& pattern : ft.extensions) {
            if (!pattern.empty() && pattern[0] == '.' && pattern == ext)
                return ft;
        }
    }

    return {"Plain Text", {}, "", "", "", {}, {}, false, false};
}

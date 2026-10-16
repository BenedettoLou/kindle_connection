#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace kindle {

std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string get_home_directory() {
#ifdef _WIN32
    const char* userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        return std::string(userprofile);
    }
    const char* homedrive = std::getenv("HOMEDRIVE");
    const char* homepath = std::getenv("HOMEPATH");
    if (homedrive && homepath) {
        return std::string(homedrive) + std::string(homepath);
    }
    return ".";
#else
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home);
    }
    return ".";
#endif
}

static std::string unescape_terminal_path(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            char next = input[i + 1];
            // Common escaped characters in terminal drag-and-drop
            if (next == ' ' || next == '(' || next == ')' || next == '[' || 
                next == ']' || next == '\'' || next == '\"' || next == '&' || 
                next == ';' || next == '$' || next == '\\') {
                result.push_back(next);
                ++i;
                continue;
            }
        }
        result.push_back(input[i]);
    }
    return result;
}

std::string clean_path(const std::string& raw_path) {
    std::string path = trim(raw_path);

    // Strip enclosing single quotes or double quotes if present
    if (path.size() >= 2) {
        if ((path.front() == '"' && path.back() == '"') ||
            (path.front() == '\'' && path.back() == '\'')) {
            path = path.substr(1, path.size() - 2);
            path = trim(path);
        }
    }

    // Strip file:// prefix if dragged from certain Linux/macOS browsers/managers
    const std::string file_proto = "file://";
    if (path.rfind(file_proto, 0) == 0) {
        path = path.substr(file_proto.length());
    }

    // Expand tilde '~' on POSIX
#ifndef _WIN32
    if (!path.empty() && path[0] == '~') {
        std::string home = get_home_directory();
        if (path.size() == 1) {
            path = home;
        } else if (path[1] == '/') {
            path = home + path.substr(1);
        }
    }
#endif

    // If the path as-is exists, return canonical/normalized path
    std::error_code ec;
    if (fs::exists(path, ec)) {
        return path;
    }

    // If it doesn't exist, try unescaping backslashes (frequent with terminal drag-and-drop on macOS/Linux)
    std::string unescaped = unescape_terminal_path(path);
    if (fs::exists(unescaped, ec)) {
        return unescaped;
    }

    // Otherwise return trimmed path for further error handling
    return path;
}

bool file_exists(const std::string& path) {
    std::error_code ec;
    return fs::exists(path, ec) && fs::is_regular_file(path, ec);
}

std::string format_file_size(const std::string& path) {
    std::error_code ec;
    uintmax_t size_bytes = fs::file_size(path, ec);
    if (ec) {
        return "Unknown size";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    if (size_bytes < 1024) {
        oss << size_bytes << " B";
    } else if (size_bytes < 1024 * 1024) {
        oss << (size_bytes / 1024.0) << " KB";
    } else if (size_bytes < 1024 * 1024 * 1024) {
        oss << (size_bytes / (1024.0 * 1024.0)) << " MB";
    } else {
        oss << (size_bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
    }
    return oss.str();
}

std::string prompt_user(const std::string& prompt_msg, const std::string& default_val) {
    std::cout << prompt_msg;
    if (!default_val.empty()) {
        std::cout << " [" << default_val << "]";
    }
    std::cout << ": ";
    std::cout.flush();

    std::string input;
    if (!std::getline(std::cin, input)) {
        return default_val;
    }
    input = trim(input);
    if (input.empty()) {
        return default_val;
    }
    return input;
}

bool prompt_confirm(const std::string& prompt_msg, bool default_yes) {
    std::cout << prompt_msg;
    if (default_yes) {
        std::cout << " [Y/n]: ";
    } else {
        std::cout << " [y/N]: ";
    }
    std::cout.flush();

    std::string input;
    if (!std::getline(std::cin, input)) {
        return default_yes;
    }
    input = trim(input);
    if (input.empty()) {
        return default_yes;
    }

    char first = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
    if (first == 'y') return true;
    if (first == 'n') return false;
    return default_yes;
}

} // namespace kindle

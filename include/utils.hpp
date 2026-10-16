#pragma once

#include <string>

namespace kindle {

// Trims leading and trailing whitespace from string
std::string trim(const std::string& str);

// Returns user's home directory (cross-platform)
std::string get_home_directory();

// Cleans up user-entered or drag-and-dropped file paths:
// - Removes enclosing quotes (' or ")
// - Handles escaped spaces and characters (\ )
// - Expands tilde (~) on POSIX
// - Resolves relative paths
std::string clean_path(const std::string& raw_path);

// Checks if a file exists and is a regular file
bool file_exists(const std::string& path);

// Formats file size into a human-readable string (e.g. 1.25 MB)
std::string format_file_size(const std::string& path);

// Prompts user with a message and optional default value
std::string prompt_user(const std::string& prompt_msg, const std::string& default_val = "");

// Prompts for a boolean confirmation [Y/n] or [y/N]
bool prompt_confirm(const std::string& prompt_msg, bool default_yes = true);

} // namespace kindle

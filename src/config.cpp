#include "config.hpp"
#include "utils.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace kindle {

std::string get_config_file_path() {
    std::string home = get_home_directory();
    fs::path conf_path = fs::path(home) / ".kindle_send.conf";
    return conf_path.string();
}

KindleSettings load_settings() {
  // getting the preloaded settings
    KindleSettings settings;
    std::string path = get_config_file_path();

    std::ifstream file(path);
    if (!file.is_open()) {
        return settings;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, eq_pos));
        std::string val = trim(line.substr(eq_pos + 1));

        if (key == "kindle_ip" && !val.empty()) {
            settings.kindle_ip = val;
        } else if (key == "kindle_port" && !val.empty()) {
            settings.kindle_port = val;
        } else if (key == "kindle_user" && !val.empty()) {
            settings.kindle_user = val;
        } else if (key == "remote_dest" && !val.empty()) {
            settings.remote_dest = val;
        }
    }

    return settings;
}

void save_settings(const KindleSettings& settings) {
  // saving latest used kindle setting
    std::string path = get_config_file_path();
    std::ofstream file(path);
    if (!file.is_open()) {
        return;
    }

    file << "# Kindle Send Configuration\n";
    file << "kindle_ip=" << settings.kindle_ip << "\n";
    file << "kindle_port=" << settings.kindle_port << "\n";
    file << "kindle_user=" << settings.kindle_user << "\n";
    file << "remote_dest=" << settings.remote_dest << "\n";
}

} // namespace kindle

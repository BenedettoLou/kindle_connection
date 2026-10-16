#pragma once

#include <string>

namespace kindle {

struct KindleSettings {
    std::string kindle_ip = "192.168.1.227";
    std::string kindle_port = "2222";
    std::string kindle_user = "root";
    std::string remote_dest = "/mnt/us/books/";
};

std::string get_config_file_path();
KindleSettings load_settings();
void save_settings(const KindleSettings& settings);

} // namespace kindle

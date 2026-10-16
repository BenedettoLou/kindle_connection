#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

#include "utils.hpp"
#include "config.hpp"
#include "process_runner.hpp"

namespace fs = std::filesystem;

void print_banner() {
    std::cout << "====================================================\n";
    std::cout << "       Kindle Book Transfer Tool (SSH / SCP)        \n";
    std::cout << "  Cross-platform: macOS | Linux | Windows           \n";
    std::cout << "====================================================\n\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Enable UTF-8 console output and input on Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    print_banner();

    // Check if SCP is installed and accessible
    if (!kindle::is_scp_installed()) {
        std::cerr << "[-] Error: 'scp' command was not found on your system.\n";
#ifdef _WIN32
        std::cerr << "    On Windows 10/11, enable OpenSSH Client in:\n";
        std::cerr << "    Settings -> Apps -> Optional features -> OpenSSH Client\n";
#else
	// openssh not installed
        std::cerr << "    Please install OpenSSH client (e.g., 'brew install openssh' or 'apt install openssh-client').\n";
#endif
        return 1;
    }

    // Load saved settings (or sensible defaults)
    kindle::KindleSettings settings = kindle::load_settings();

    // Check if a file was provided as a command-line argument
    std::string prefilled_file = "";
    if (argc > 1) {
        prefilled_file = kindle::clean_path(argv[1]);
    }

    bool keep_running = true;

    while (keep_running) {
        std::cout << "[*] Configure transfer details (press Enter to accept [defaults]):\n\n";

        // 1. Ask for Kindle IP
        std::string ip_input = kindle::prompt_user("Kindle IP address", settings.kindle_ip);
        if (ip_input.empty()) {
            std::cerr << "[-] IP address cannot be empty.\n";
            continue;
        }
        settings.kindle_ip = ip_input;

        // 2. Ask for SSH Port
        std::string port_input = kindle::prompt_user("Kindle SSH Port", settings.kindle_port);
        if (port_input.empty()) {
            port_input = "2222";
        }
        settings.kindle_port = port_input;

        // 3. Ask for SSH Username
        std::string user_input = kindle::prompt_user("Kindle SSH Username", settings.kindle_user);
        if (user_input.empty()) {
            user_input = "root";
        }
        settings.kindle_user = user_input;

        // 4. Ask for File Path (with retry if file not found)
        std::string file_path = prefilled_file;
        prefilled_file.clear(); // Only use once

        while (true) {
            if (file_path.empty()) {
                std::cout << "\nEnter book file path (drag and drop file here):\n> ";
                std::string raw_input;
                if (!std::getline(std::cin, raw_input)) {
                    std::cout << "\nOperation aborted.\n";
                    return 0;
                }
                file_path = kindle::clean_path(raw_input);
            }

            if (file_path.empty()) {
                std::cout << "[-] Path cannot be empty. Please enter a valid file.\n";
                continue;
            }

            if (!kindle::file_exists(file_path)) {
                std::cout << "[-] File not found: " << file_path << "\n";
                std::cout << "    Please verify the path or drag & drop the file again.\n";
                file_path.clear();
                continue;
            }

            break;
        }

        fs::path p(file_path);
        std::cout << "[+] Selected file: " << p.filename().string() << "\n";
        std::cout << "    Size: " << kindle::format_file_size(file_path) << "\n\n";

        // 5. Ask for Destination path on Kindle
        std::string dest_input = kindle::prompt_user("Destination path on Kindle", settings.remote_dest);
        if (dest_input.empty()) {
            dest_input = "/mnt/us/books/";
        }
        // Ensure trailing slash if it points to a directory
        if (!dest_input.empty() && dest_input.back() != '/') {
            // If user did not specify a filename in the destination, append '/'
            if (dest_input.find('.') == std::string::npos) {
                dest_input += "/";
            }
        }
        settings.remote_dest = dest_input;

        // Save settings for next session
        kindle::save_settings(settings);

        // Display summary
        std::cout << "\n----------------------------------------------------\n";
        std::cout << "Transfer Summary:\n";
        std::cout << "  File:        " << file_path << "\n";
        std::cout << "  Target:      " << settings.kindle_user << "@" << settings.kindle_ip 
                  << ":" << settings.kindle_port << "\n";
        std::cout << "  Destination: " << settings.remote_dest << "\n";
        std::cout << "----------------------------------------------------\n";

        if (!kindle::prompt_confirm("Start copying to Kindle?")) {
            std::cout << "[!] Transfer cancelled.\n";
        } else {
            std::cout << "\n[*] Starting scp transfer...\n";
            std::cout << "----------------------------------------------------\n";

            int result = kindle::execute_scp(
                settings.kindle_port,
                file_path,
                settings.kindle_user,
                settings.kindle_ip,
                settings.remote_dest
            );

            std::cout << "----------------------------------------------------\n";
            if (result == 0) {
                std::cout << "\n[+] SUCCESS: Book was transferred to your Kindle!\n";
                std::cout << "    Tip: If using KOReader, open the folder or rescan library.\n";
                std::cout << "    If using standard Kindle UI, you may need to restart the UI\n";
                std::cout << "    or run 'killall -HUP cvm' to refresh the native catalog.\n";
            } else {
                std::cout << "\n[-] ERROR: Transfer failed (exit code " << result << ").\n";
                std::cout << "    Troubleshooting checklist:\n";
                std::cout << "    1. Is your Kindle awake and connected to the same Wi-Fi / USB network?\n";
                std::cout << "    2. Is SSH running on your Kindle (KUAL -> USBNetwork / SSH)?\n";
                std::cout << "    3. Is Kindle IP " << settings.kindle_ip << " reachable? (try: ping " << settings.kindle_ip << ")\n";
                std::cout << "    4. Is SSH listening on port " << settings.kindle_port << "?\n";
                std::cout << "    5. Verify the destination directory exists on Kindle.\n";
            }
        }

        std::cout << "\n";
        keep_running = kindle::prompt_confirm("Do you want to send another book?", false);
        std::cout << "\n";
    }

    std::cout << "--END--\n";
    return 0;
}

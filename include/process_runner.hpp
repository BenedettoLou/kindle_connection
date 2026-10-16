#pragma once

#include <string>
#include <vector>

namespace kindle {

// Checks whether scp is available on the current system PATH
bool is_scp_installed();

// Runs a command interactively, allowing stdin/stdout/stderr to remain connected to terminal
int run_process_interactive(const std::string& program, const std::vector<std::string>& args);

// High-level wrapper to invoke scp safely without shell injection
// Matches command structure: scp -P <port> "<local_file>" <user>@<ip>:<remote_dest>
int execute_scp(
    const std::string& port,
    const std::string& local_file,
    const std::string& user,
    const std::string& ip,
    const std::string& remote_dest
);

} // namespace kindle

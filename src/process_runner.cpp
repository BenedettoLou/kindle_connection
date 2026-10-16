#include "process_runner.hpp"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

namespace kindle {

bool is_scp_installed() {
    const char* path_env = std::getenv("PATH");
    if (!path_env) {
        return false;
    }

#ifdef _WIN32
    // Check known OpenSSH directory on Windows first
    if (fs::exists("C:\\Windows\\System32\\OpenSSH\\scp.exe")) {
        return true;
    }

    std::stringstream ss(path_env);
    std::string item;
    while (std::getline(ss, item, ';')) {
        if (item.empty()) continue;
        fs::path p = fs::path(item) / "scp.exe";
        if (fs::exists(p)) {
            return true;
        }
    }
    return false;
#else
    std::stringstream ss(path_env);
    std::string item;
    while (std::getline(ss, item, ':')) {
        if (item.empty()) continue;
        fs::path p = fs::path(item) / "scp";
        if (fs::exists(p) && access(p.c_str(), X_OK) == 0) {
            return true;
        }
    }
    return false;
#endif
}

#ifdef _WIN32

static std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &wstr[0], size_needed);
    return wstr;
}

static std::wstring quote_win_arg(const std::wstring& arg) {
    if (arg.empty()) return L"\"\"";
    if (arg.find_first_of(L" \t\n\v\"") == std::wstring::npos) {
        return arg;
    }
    std::wstring result = L"\"";
    for (size_t i = 0; i < arg.length(); ++i) {
        size_t backslash_count = 0;
        while (i < arg.length() && arg[i] == L'\\') {
            ++backslash_count;
            ++i;
        }
        if (i == arg.length()) {
            result.append(backslash_count * 2, L'\\');
            break;
        } else if (arg[i] == L'"') {
            result.append(backslash_count * 2 + 1, L'\\');
            result.push_back(L'"');
        } else {
            result.append(backslash_count, L'\\');
            result.push_back(arg[i]);
        }
    }
    result.push_back(L'"');
    return result;
}

int run_process_interactive(const std::string& program, const std::vector<std::string>& args) {
    std::wstring cmdline = quote_win_arg(utf8_to_wstring(program));
    for (const auto& a : args) {
        cmdline += L" ";
        cmdline += quote_win_arg(utf8_to_wstring(a));
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::vector<wchar_t> cmdline_buf(cmdline.begin(), cmdline.end());
    cmdline_buf.push_back(0);

    // Create process inheriting standard input/output/error of console
    BOOL success = CreateProcessW(
        NULL,
        cmdline_buf.data(),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!success) {
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(exitCode);
}

#else

int run_process_interactive(const std::string& program, const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        // In child process: Prepare arguments for execvp
        std::vector<char*> c_argv;
        c_argv.push_back(const_cast<char*>(program.c_str()));
        for (const auto& a : args) {
            c_argv.push_back(const_cast<char*>(a.c_str()));
        }
        c_argv.push_back(nullptr);

        execvp(program.c_str(), c_argv.data());
        // If execvp failed:
        _exit(127);
    }

    // In parent process: Wait for child to finish
    int status = 0;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return status;
}

#endif

int execute_scp(
    const std::string& port,
    const std::string& local_file,
    const std::string& user,
    const std::string& ip,
    const std::string& remote_dest
) {
    std::string destination_arg = user + "@" + ip + ":" + remote_dest;
    std::vector<std::string> args = {
        "-P",
        port,
        local_file,
        destination_arg
    };

    return run_process_interactive("scp", args);
}

} // namespace kindle

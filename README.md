# Kindle Connection - Cross-Platform Book Transfer Tool

A modern C++ application to safely transfer books and documents to an SSH-connected Kindle (via Wi-Fi or USBNetwork) using `scp`. 

Works natively on **macOS**, **Linux**, and **Windows**.

---

## Features

- **Safe Execution**: Uses native OS process APIs (`fork`/`execvp` on POSIX, `CreateProcessW` on Windows) rather than unsafe shell calls. Filenames with spaces, quotes, brackets, Unicode, and special characters (like `--`, `,`, `&`, `_`) transfer without syntax errors or escaping issues.
- **Drag-and-Drop Friendly**: Drag a file directly into the terminal window. Automatically strips quotes, resolves escaped spaces (`\ `), and expands `~`.
- **Interactive Prompts with Defaults**:
  - **Kindle IP address** (e.g. `192.168.1.227` or `192.168.15.244`)
  - **SSH Port** (default: `2222`, standard for Kindle USBNet / OpenSSH)
  - **SSH Username** (default: `root`)
  - **Local Book File** (verifies that the file exists and shows file size)
  - **Destination Path on Kindle** (default: `/mnt/us/books/`)
- **Remembers Settings**: Saves your IP, port, user, and target folder in `~/.kindle_send.conf` so subsequent runs let you press `Enter` to accept defaults.
- **Full SSH Interactivity**: SSH password prompts, fingerprint verification, and `scp` transfer progress bars display seamlessly.
- **Optional CLI Argument**: You can run it interactively or pass a file directly:
  ```bash
  ./kindle_send "path/to/book.epub"
  ```

---

## Prerequisites

1. **C++17 Compiler & CMake**:
   - macOS: Apple Clang (`xcode-select --install`) and `cmake` (`brew install cmake`).
   - Linux: GCC or Clang (`sudo apt install build-essential cmake`).
   - Windows: Visual Studio 2019/2022 (with "Desktop development with C++") or MinGW-w64.
2. **OpenSSH Client (`scp`)**:
   - macOS & Linux: Pre-installed.
   - Windows 10/11: Built-in OpenSSH client (`Settings` > `System` > `Optional features` > `OpenSSH Client`).

---

## Build Instructions

### macOS / Linux

```bash
# Generate build files
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the executable
cmake --build build

# Run the application
./build/kindle_send
```

*(Optional) Install globally to `/usr/local/bin`:*
```bash
sudo cmake --install build
```

---

### Windows

Using Developer Command Prompt or PowerShell:

```powershell
# Generate Visual Studio solution
cmake -B build

# Build Release binary
cmake --build build --config Release

# Run the executable
.\build\Release\kindle_send.exe
```

---

## Example Usage

```
====================================================
       Kindle Book Transfer Tool (SSH / SCP)        
  Cross-platform: macOS | Linux | Windows           
====================================================

[*] Configure transfer details (press Enter to accept [defaults]):

Kindle IP address [192.168.1.227]: 
Kindle SSH Port [2222]: 
Kindle SSH Username [root]: 

Enter book file path (drag and drop file here):
> "C++ Software Design _ Design Principles and Patterns for -- Klaus Iglberger -- 1, 2022.epub"
[+] Selected file: C++ Software Design _ Design Principles and Patterns for -- Klaus Iglberger -- 1, 2022.epub
    Size: 2.45 MB

Destination path on Kindle [/mnt/us/books/]: 

----------------------------------------------------
Transfer Summary:
  File:        C++ Software Design _ Design Principles and Patterns for -- Klaus Iglberger -- 1, 2022.epub
  Target:      root@192.168.1.227:2222
  Destination: /mnt/us/books/
----------------------------------------------------
Start copying to Kindle? [Y/n]: Y

[*] Starting scp transfer...
----------------------------------------------------
C++ Software Design... 100% 2.45MB   1.2MB/s   00:02
----------------------------------------------------

[+] SUCCESS: Book was transferred to your Kindle!
```

---

## Kindle SSH Tips

1. **Jailbreak / USBNetwork**: Ensure USBNetwork or OpenSSH is active on your Kindle (commonly installed via KUAL / Mobileread package).
2. **Port**:
   - WiFi SSH is typically on port `2222` or `22`.
   - USBNet over cable is typically IP `192.168.15.244` on port `22`.
3. **Library Refresh**:
   - **KOReader**: Files transferred to `/mnt/us/books/` or `/mnt/us/documents/` are immediately visible in the file manager.
   - **Native Kindle UI**: The native catalog indexer (`cvm`) scans documents upon restart. You can either reboot or restart the framework via SSH:
     ```bash
     killall -HUP cvm
     ```

## PS
I've set an alternative preset folder such as _books_ because having books in _documents_ was causing me indexing problems.
Your are free to set the folder you want.

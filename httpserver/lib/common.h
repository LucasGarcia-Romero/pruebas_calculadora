#pragma once

// Cross-platform common includes and helpers for the HTTP server (Linux/ARM64 + Windows)

#include <cstdint>
#include <cstring>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <vector>
#include <map>
#include <list>

#include <mutex>
#include <thread>
#include <chrono>

// Sockets
#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <signal.h>
#endif

// Provide Windows-style integer aliases used in the original code when compiling on Linux
#ifndef _WIN32
  using __int64 = int64_t;
  using ULONG  = uint32_t;
#endif

// Allow `100ms` literals, etc.
using namespace std::chrono_literals;

// Convenience: the original code uses `string` without `std::`
using std::string;

#if defined(__has_include)
  #if __has_include(<png.h>)
    #include <png.h>
  #endif
#endif

// ---------------------------
// Small utility helpers used across the server code
// (Some repos had these in other files; we keep them here header-only
// to avoid "not declared in this scope" during Linux builds.)
// ---------------------------

inline void replaceSubstrs(std::string& s, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
    }
}

// Read file into a std::string (binary safe)
inline std::string readFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return {};
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

// Basic percent-decoding for application/x-www-form-urlencoded style bodies.
// Also converts '+' to space.
inline std::string decodeURIComponent(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    auto hexval = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        return -1;
    };

    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < s.size()) {
            int hi = hexval(s[i + 1]);
            int lo = hexval(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else {
            out.push_back(c);
        }
    }
    return out;
}

// #define DEBUG_APP

//
// Created by dawid on 30.11.2021.
//

#include "Utils.h"


std::string ult::get_executable_name() {
#if __linux__
    return program_invocation_name;
#endif
#if _WIN32
    //  TODO: WINDOWS SUPPORT
#endif
}


std::string ult::get_home_path() {
#if __linux__
    return getenv("HOME");
#endif
#if _WIN32
    return getenv("USERPROFILE");
#endif
}

bool ult::is_linux() {
#if __linux__
    return true;
#else
    return false;
#endif
}

bool ult::is_windows() {
#if _WIN32
    return true;
#else
    return false;
#endif
}

bool ult::is_elevated() {
#if __linux__
    return !getuid();
#endif
#if _WIN32
    return // TODO: windows support
#endif
}

std::string ult::to_lower(const std::string &str) {
    std::string result;
    for (auto c : str)
        result += (char)std::tolower(c);
    return result;
}
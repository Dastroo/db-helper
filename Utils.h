//
// Created by dawid on 05.11.2021.
//

#pragma once


#include <iostream>
#include <unistd.h>

/// UtiLiTy
namespace ult {
    std::string get_executable_name();

    std::string get_home_path();

    bool is_linux();

    bool is_windows();

    bool is_elevated();

    template<typename S, typename ... Args>
    std::string concatenate(const S &s, Args &&... args) {
        return ((std::string) s += ... += args);
    }

    template<typename S, typename ...Args>
    std::string intersect(const S &s, Args &&...args) {
        return ((std::string) s += ... += ((std::string) ", " += args));
    }

    template<typename S, typename ...Args>
    std::string intersect_with_colon(const S &s, Args &&...args) {
        return '\"' + ((std::string) s += ... += ((std::string) "\", \"" += args)) + '\"';
    }

    std::string to_lower(const std::string &str);

    template<typename Base, typename T>
    inline bool instanceof(const T) {
        return std::is_base_of<Base, T>::value;
    }
}
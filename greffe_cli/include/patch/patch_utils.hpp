#pragma once
#include <string>
#include <cctype>

inline std::string sanitize(const std::string& name) {
    std::string s;
    for (char c : name)
        s += (std::isalnum(static_cast<unsigned char>(c)) || c == '_') ? c : '_';
    return s;
}

#pragma once

#include "colors.hpp"
#include <iostream>
#include <string_view>

inline void cli_error(std::string_view msg) {
    std::cerr << Color::GREY << msg << Color::RST << '\n';
}

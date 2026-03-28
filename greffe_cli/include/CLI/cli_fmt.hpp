#pragma once

#include "colors.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <readline/readline.h>

inline void cli_error(std::string_view msg) {
    std::cerr << Color::GREY << msg << Color::RST << '\n';
}

// Print from a background thread without corrupting the readline prompt.
inline void async_print(const std::string& msg) {
    rl_save_prompt();
    rl_clear_visible_line();
    std::cout << msg;
    rl_restore_prompt();
    rl_forced_update_display();
}

inline bool prompt_confirm(const std::string& msg) {
    char* raw = readline(msg.c_str());
    std::string answer;
    if (raw) { answer = raw; free(raw); }
    return answer == "y" || answer == "Y";
}

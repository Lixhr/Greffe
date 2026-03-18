#include "CompletionRegistry.hpp"

#include <readline/readline.h>
#include <cstring>

CompletionRegistry& CompletionRegistry::instance() {
    static CompletionRegistry reg;
    return reg;
}

void CompletionRegistry::install(const CLIDispatcher& dispatcher) {
    _dispatcher = &dispatcher;
    rl_attempted_completion_function = &CompletionRegistry::readline_completion;
}

char** CompletionRegistry::readline_completion(const char* text,
                                               int         start,
                                               int       /*end*/) {
    rl_attempted_completion_over = 1;

    auto& reg = instance();
    if (!reg._dispatcher) return nullptr;

    std::string line(rl_line_buffer, static_cast<std::size_t>(start));
    line += text;

    reg._candidates = reg._dispatcher->complete(line);
    reg._idx        = 0;

    if (reg._candidates.empty()) return nullptr;

    return rl_completion_matches(text, &CompletionRegistry::completion_generator);
}

char* CompletionRegistry::completion_generator(const char* /*text*/, int state) {
    auto& reg = instance();
    if (state == 0) reg._idx = 0;
    if (reg._idx >= reg._candidates.size()) return nullptr;

    return strdup(reg._candidates[reg._idx++].c_str());
}

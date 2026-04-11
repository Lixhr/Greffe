#include "CLIDispatcher.hpp"
#include "cli_fmt.hpp"

#include <sstream>

void CLIDispatcher::register_command(std::shared_ptr<ICommand> cmd) {
    std::string key(cmd->name());
    _commands[std::move(key)] = std::move(cmd);
}


void CLIDispatcher::dispatch(CLIContext& ctx, const std::string& line) const {
    auto tokens = tokenize(line);
    if (tokens.empty()) return;

    auto it = _commands.find(tokens[0]);
    if (it == _commands.end()) {
        cli_error("unknown command: " + tokens[0] + " (try 'help')");
        return;
    }

    Args tail(tokens.begin() + 1, tokens.end());
    try {
        it->second->execute(ctx, tail);
    } catch (const std::exception& e) {
        cli_error(tokens[0] + ": " + e.what());
    }
}

std::vector<std::string>
CLIDispatcher::complete(const CLIContext *ctx, const std::string& line) const {
    auto tokens = tokenize(line);

    if (tokens.empty() || (tokens.size() == 1 && !ends_with_space(line))) {
        const std::string prefix = tokens.empty() ? "" : tokens[0];
        std::vector<std::string> result;
        for (const auto& [name, _] : _commands)
            if (name.rfind(prefix, 0) == 0)
                result.push_back(name);
        return result;
    }

    auto it = _commands.find(tokens[0]);
    if (it == _commands.end()) return {};

    std::string sub;
    for (std::size_t i = 1; i < tokens.size(); ++i) {
        if (i > 1) sub += ' ';
        sub += tokens[i];
    }
    if (ends_with_space(line)) sub += ' ';

    Args tail(tokens.begin() + 1, tokens.end());
    return it->second->complete(ctx, tail);
}

const std::unordered_map<std::string, std::shared_ptr<ICommand>>&
CLIDispatcher::commands() const {
    return _commands;
}

std::vector<std::string> CLIDispatcher::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string tok;
    while (iss >> tok)
        tokens.push_back(std::move(tok));
    return tokens;
}

bool CLIDispatcher::ends_with_space(const std::string& s) {
    return !s.empty() && s.back() == ' ';
}

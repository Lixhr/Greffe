#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ICommand.hpp"
#include "CLIContext.hpp"

class CLIDispatcher {
    public:
        void                            register_command(std::shared_ptr<ICommand> cmd);
        void                            dispatch(CLIContext& ctx, const std::string& line) const;
        static std::vector<std::string> tokenize(const std::string& line);
        std::vector<std::string>        complete(const CLIContext *ctx, const std::string& line) const;

        const std::unordered_map<std::string, std::shared_ptr<ICommand>>&
        commands() const;

    private:
        static bool                     ends_with_space(const std::string& s);
        std::unordered_map<std::string, std::shared_ptr<ICommand>> _commands;
};

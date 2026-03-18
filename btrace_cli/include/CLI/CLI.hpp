#pragma once

#include "CLIContext.hpp"
#include "CLIDispatcher.hpp"
#include "CompletionRegistry.hpp"

struct CommandNode {
    std::string node_name;
    std::string node_desc;
    std::shared_ptr<ICommand>   cmd;
    std::vector<CommandNode>    children;
};

CommandNode leaf(std::unique_ptr<ICommand> cmd);
CommandNode group(std::string name, std::string desc,
                  std::vector<CommandNode> children);

class CLI {
    public:
        explicit CLI(CLIContext& ctx);

        void register_command(std::unique_ptr<ICommand> cmd);

        void run();

    private:
        void register_tree(std::vector<CommandNode> tree); 
        CLIContext&   _ctx;
        CLIDispatcher _dispatcher;
};

#include "BuiltinCommands.hpp"
#include "CompositeCommand.hpp"

#include <iostream>
#include <algorithm>
#include <vector>

HelpCommand::HelpCommand(const CLIDispatcher& dispatcher)
    : _dispatcher(dispatcher) {}

std::string_view HelpCommand::name()        const { return "help"; }
std::string_view HelpCommand::description() const { return "Show this help message"; }

void HelpCommand::execute(CLIContext& /*ctx*/, const Args& /*args*/) {
    std::cout << '\n';
    print_dispatcher(_dispatcher, "", "");
    std::cout << '\n';
}


void HelpCommand::print_dispatcher(const CLIDispatcher& dispatcher,
                                   const std::string&   indent,
                                   const std::string&   prefix) {
    std::vector<const ICommand*> sorted;
    for (const auto& [_, cmd] : dispatcher.commands())
        sorted.push_back(cmd.get());
    std::sort(sorted.begin(), sorted.end(),
              [](const ICommand* a, const ICommand* b){
                  return a->name() < b->name();
              });

    constexpr int COL = 28;

    for (const ICommand* cmd : sorted) {
        std::string full_name = prefix + std::string(cmd->name());

        std::cout << indent << full_name;

        int pad = COL - static_cast<int>(indent.size())
                      - static_cast<int>(full_name.size());
        if (pad > 0)
            std::cout << std::string(pad, ' ');
        else
            std::cout << "  ";

        std::cout << cmd->description() << '\n';

        const auto* composite = dynamic_cast<const CompositeCommand*>(cmd);
        if (composite)
            print_dispatcher(composite->dispatcher(),
                             indent + "  ",
                             "");
    }
}

std::string_view QuitCommand::name()        const { return "quit"; }
std::string_view QuitCommand::description() const { return "Exit the CLI"; }

void QuitCommand::execute(CLIContext& ctx, const Args& /*args*/) {
    ctx.running = false;
}
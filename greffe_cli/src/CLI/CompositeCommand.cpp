#include "CompositeCommand.hpp"

#include <iostream>

CompositeCommand::CompositeCommand(std::string name, std::string description)
    : _name(std::move(name))
    , _description(std::move(description)) {}

std::string_view CompositeCommand::name()        const { return _name; }
std::string_view CompositeCommand::description() const { return _description; }

CLIDispatcher&       CompositeCommand::dispatcher()       { return _dispatcher; }
const CLIDispatcher& CompositeCommand::dispatcher() const { return _dispatcher; }

void CompositeCommand::add(std::unique_ptr<ICommand> sub) {
    _dispatcher.register_command(std::move(sub));
}

void CompositeCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.empty()) {
        print_usage();
        return;
    }

    std::string sub_line;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) sub_line += ' ';
        sub_line += args[i];
    }
    _dispatcher.dispatch(ctx, sub_line);
}

std::vector<std::string>
CompositeCommand::complete(const CLIContext *ctx, const Args& args) const {
    std::string sub_line;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) sub_line += ' ';
        sub_line += args[i];
    }
    return _dispatcher.complete(ctx, sub_line);
}

void CompositeCommand::print_usage() const {
    std::cout << _name << " — " << _description << "\n\nSubcommands:\n";
    for (const auto& [name, cmd] : _dispatcher.commands())
        std::cout << "  " << name << "\t\t" << cmd->description() << '\n';
    std::cout << '\n';
}
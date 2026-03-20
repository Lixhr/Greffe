#include "CLI.hpp"

#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>
#include "CompositeCommand.hpp"
#include "BuiltinCommands.hpp"
#include "TargetCommands.hpp"
#include "cli_fmt.hpp"
#include "colors.hpp"

CLI::CLI(CLIContext& ctx) : _ctx(ctx) {
    CompletionRegistry::instance().install(_dispatcher);
    register_tree({
    leaf(std::make_unique<AddCommand>()),
    leaf(std::make_unique<DelCommand>()),
    leaf(std::make_unique<ListCommand>()),
    leaf(std::make_unique<SaveCommand>()),
    leaf(std::make_unique<PatchCommand>()),
    leaf(std::make_unique<QuitCommand>()),



});
}

static void fill_dispatcher(CLIDispatcher& dispatcher,
                             std::vector<CommandNode> tree) {
    for (auto& node : tree) {
        if (node.cmd) {
            dispatcher.register_command(std::move(node.cmd));
        } else {
            auto composite = std::make_unique<CompositeCommand>(
                node.node_name, node.node_desc);

            fill_dispatcher(composite->dispatcher(), 
                std::move(node.children));

            dispatcher.register_command(std::move(composite));
        }
    }
}

void CLI::register_command(std::unique_ptr<ICommand> cmd) {
    _dispatcher.register_command(std::move(cmd));
}

void CLI::register_tree(std::vector<CommandNode> tree) {
    fill_dispatcher(_dispatcher, std::move(tree));

    _dispatcher.register_command(std::make_unique<HelpCommand>(_dispatcher));
}


CommandNode leaf(std::unique_ptr<ICommand> cmd) {
    CommandNode n;
    n.cmd = std::move(cmd);
    return n;
}

CommandNode group(std::string name, std::string desc,
                  std::vector<CommandNode> children) {
    CommandNode n;
    n.node_name = std::move(name);
    n.node_desc = std::move(desc);
    n.children  = std::move(children);
    return n;
}

static void print_banner() {
    using namespace Color;
    std::cout
        << BLUE << "  ╔══════════════════════════════════════╗\n"
        << BLUE << "  ║  " << BOLD << "btrace" << RST << BLUE << "                              ║\n"
        << BLUE << "  ║  " << DIM  << "Binary Tracing & Analysis CLI" << RST << BLUE << "       ║\n"
        << BLUE << "  ╚══════════════════════════════════════╝\n"
        << RST
        << CYAN << "  Type " << BOLD << "help" << RST << CYAN << " to list available commands.\n"
        << RST << '\n';
}

void CLI::run() {
    print_banner();

    static constexpr auto prompt = "\001\033[34m\002btrace>\001\033[0m\002 ";
    char* raw = nullptr;
    while (_ctx.running && (raw = readline(prompt)) != nullptr) {
        if (*raw) {
            add_history(raw);
            try {
                _dispatcher.dispatch(_ctx, raw);
            }
            catch (const std::exception& e) {
                cli_error(e.what());
            }
        }
        free(raw);
    }
    std::cout << '\n';
}

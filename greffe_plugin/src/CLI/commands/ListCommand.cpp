#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "cli_fmt.hpp"
#include "colors.hpp"
#include <iostream>

std::string_view ListCommand::name()        const { return "list"; }
std::string_view ListCommand::description() const { return "List registered greffes"; }

void ListCommand::execute(CLIContext& ctx, const Args&) {
    const auto& plans = ctx.targets.plans();

    if (plans.empty()) {
        std::cout << Color::DIM << "no targets registered" << Color::RST << '\n';
        return;
    }

    int bits = ctx.pinfo.getBits();
    for (size_t i = 0; i < plans.size(); ++i) {
        std::cout << Color::BOLD << "[" << std::setfill('0') << std::setw(2) << i << "] " \
        << TargetView{ plans[i].target, bits };
    }
}

#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "colors.hpp"
#include <iostream>

std::string_view DelCommand::name()        const { return "del"; }
std::string_view DelCommand::description() const { return "Remove a greffe"; }

void DelCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.empty())
        throw std::runtime_error("usage: del <target> [target ...]");

    for (const auto& arg : args) {
        ctx.targets.remove(arg);
        std::cout << Color::GREY << "removed " << arg << Color::RST << '\n';
    }
}

std::vector<std::string>
DelCommand::complete(const CLIContext* ctx, const Args& args) const {
    std::vector<std::string> result;

    if (!ctx)
        return result;
    const auto& plans = ctx->targets.plans();
    const std::string prefix = args.empty() ? "" : args.back();

    for (const auto& p : plans) {
        const std::string& name = p.target.name();

        if (name.rfind(prefix, 0) == 0) {
            result.push_back(name);
        }
    }

    return result;
}

#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "colors.hpp"
#include <algorithm>
#include <iostream>

std::string_view DelCommand::name()        const { return "del"; }
std::string_view DelCommand::description() const { return "Remove a greffe"; }

void DelCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.empty())
        throw std::runtime_error("usage: del <index> [index ...]");

    std::vector<std::string> sorted(args.begin(), args.end());
    std::stable_sort(sorted.begin(), sorted.end(), [](const std::string& a, const std::string& b) {
        // non-numeric args sort first; remove() will reject them with a clear error
        bool a_num = !a.empty() && std::all_of(a.begin(), a.end(), ::isdigit);
        bool b_num = !b.empty() && std::all_of(b.begin(), b.end(), ::isdigit);
        if (a_num && b_num) return std::stoull(a) > std::stoull(b);
        return !a_num && b_num;
    });

    for (const auto& arg : sorted) {
        ctx.targets.remove(arg, ctx);
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

    for (size_t i = 0; i < plans.size(); ++i) {
        const std::string idx = std::to_string(i);
        if (idx.rfind(prefix, 0) == 0)
            result.push_back(idx);
    }

    return result;
}

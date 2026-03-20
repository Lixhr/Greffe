#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "Target.hpp"
#include "HandlerBin.hpp"
#include "PatchSession.hpp"
#include "cli_fmt.hpp"
#include "colors.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>

std::string_view AddCommand::name()        const { return "add"; }
std::string_view AddCommand::description() const { return "Add one or more trace targets"; }

void AddCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.empty()) {
        cli_error("usage: add <target> [target ...]");
        return;
    }

    for (const auto& arg : args) {
        try {
            std::cout << TargetView{ ctx.targets.add(arg), ctx.pinfo.getBits() };
        } catch (const std::exception& e) {
            cli_error(std::string(arg) + ": " + e.what());
        }
    }
}

std::string_view DelCommand::name()        const { return "del"; }
std::string_view DelCommand::description() const { return "Remove one or more trace targets"; }

void DelCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.empty()) {
        cli_error("usage: del <target> [target ...]");
        return;
    }

    for (const auto& arg : args) {
        try {
            ctx.targets.remove(arg);
            std::cout << Color::GREY << "removed " << arg << Color::RST << '\n';
        } catch (const std::exception& e) {
            cli_error(std::string(arg) + ": " + e.what());
        }
    }
}

std::string_view SaveCommand::name()        const { return "save"; }
std::string_view SaveCommand::description() const { return "Save targets to workdir/.btrace"; }

void SaveCommand::execute(CLIContext& ctx, const Args&) {
    auto path = ctx.pinfo.getProjectDir() / ".btrace";
    ctx.targets.save(path);
    std::cout << Color::GREY << "saved to " << path.string() << Color::RST << '\n';
}

std::string_view ListCommand::name()        const { return "list"; }
std::string_view ListCommand::description() const { return "List all registered targets"; }

void ListCommand::execute(CLIContext& ctx, const Args&) {
    const auto& targets = ctx.targets.targets();

    if (targets.empty()) {
        std::cout << Color::DIM << "no targets registered" << Color::RST << '\n';
        return;
    }

    int bits = ctx.pinfo.getBits();
    for (const auto& t : targets)
        std::cout << TargetView{ t, bits };
}

std::string_view PatchCommand::name()        const { return "patch"; }
std::string_view PatchCommand::description() const { return "Patch all registered targets: write trampolines and hooks to the binary"; }

void PatchCommand::execute(CLIContext& ctx, const Args&) {
    if (ctx.targets.targets().empty()) {
        cli_error("nothing to patch");
        return;
    }

    auto outfile = std::filesystem::path(ctx.pinfo.getBinPath().string() + ".patched_");
    HandlerBin handler_bin;

    try {
        PatchSession::run(ctx.targets.targets(), handler_bin,
                          ctx.patch_base, ctx.bin_base,
                          ctx.pinfo, outfile);
    } catch (const std::exception& e) {
        cli_error(e.what());
    }
}

#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "HandlerCompiler.hpp"
#include "PatchSession.hpp"
#include "cli_fmt.hpp"
#include "colors.hpp"
#include <filesystem>
#include <iomanip>
#include <iostream>

std::string_view PatchCommand::name()        const { return "patch"; }
std::string_view PatchCommand::description() const { return "Compile handlers and apply all greffes"; }

const std::filesystem::path PatchCommand::get_output_path(CLIContext& ctx) const {
    auto out_filename = ctx.pinfo.getBinPath().filename();
    out_filename += ".greffe";

    auto out_path = ctx.pinfo.getProjectDir() / out_filename;
    return (out_path);
}

bool PatchCommand::confirm_output(const std::filesystem::path &out_path) const {
    if (std::filesystem::exists(out_path))
        return prompt_confirm(out_path.string() + " already exists. Overwrite? [y/N]");
    return prompt_confirm("Proceed? [y/N]");
}

void PatchCommand::print_patch_info(CLIContext& ctx) {
    int      bits = ctx.pinfo.getBits();
    unsigned w    = static_cast<unsigned>(bits / 4);

    std::cout << "  bin_base   : 0x" << std::hex << std::setfill('0')
              << std::setw(w) << ctx.bin_base                    << '\n'
              << "  patch_base : 0x" << std::setw(w) << ctx.pinfo.getPatchBase()
              << std::dec << '\n';
}

void PatchCommand::execute(CLIContext& ctx, const Args&) {
    if (ctx.targets.targets().empty())
        throw std::runtime_error("nothing to patch");

    print_patch_info(ctx);

    auto out_path = get_output_path(ctx);
    if (!confirm_output(out_path)) {
        std::cout << Color::GREY << "aborted" << Color::RST << '\n';
        return;
    }

    HandlerBin handler_bin = HandlerCompiler::build(ctx.targets.targets(), ctx.pinfo);
    PatchSession::run(ctx.targets.targets(), handler_bin,
                      ctx.pinfo.getPatchBase(), ctx.bin_base,
                      ctx.pinfo, out_path);
}

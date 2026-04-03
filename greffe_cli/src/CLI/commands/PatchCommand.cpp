#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "HandlerCompiler.hpp"
#include "TrampolineBuilder.hpp"
#include "PatchSession.hpp"
#include "patch/patch_utils.hpp"
#include "cli_fmt.hpp"
#include "colors.hpp"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>

std::string_view PatchCommand::name()        const { return "patch"; }
std::string_view PatchCommand::description() const { return "Compile handlers and apply all greffes"; }

const std::filesystem::path PatchCommand::get_output_path(CLIContext& ctx) const {
    auto out_filename = ctx.pinfo.getBinPath().filename();
    out_filename += ".greffe";

    auto out_path = ctx.pinfo.getProjectDir() / out_filename;
    return out_path;
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
    if (ctx.targets.plans().empty())
        throw std::runtime_error("nothing to patch");

    print_patch_info(ctx);

    const std::filesystem::path out_path = get_output_path(ctx);
    if (!confirm_output(out_path)) {
        std::cout << Color::GREY << "aborted" << Color::RST << '\n';
        return;
    }

    PatchSession session(ctx.pinfo.getBinPath(), ctx.bin_base);

    // compiled blob
    HandlerBin handler_bin = HandlerCompiler::build(ctx.targets.plans(), ctx.pinfo);
    handler_bin.set_offset(ctx.layout.current_offset());
    handler_bin.set_addr(ctx.layout.offset_to_addr(ctx.layout.current_offset()));

    // write handler addresses into each trampoline's fake literal pool
    for (auto& plan : ctx.targets.plans()) {
        std::string sym = "handler_" + sanitize(plan.target.name());
        uint64_t handler_addr = handler_bin.handler_addr(sym);
        uint8_t* slot = plan.bytes().data() + plan.handler_offset;
        plan.stubs->write_ptr(slot, handler_addr);
    }

    // point targets to trampolines
    TrampolineBuilder::patch_branches(session, ctx.targets.plans());

    auto patch_entry = [&](const PatchLayoutEntry& e) {
        session.patch(e.addr(), e.bytes());
    };

    for (const auto& plan   : ctx.layout.patch_plans()) patch_entry(plan);
    for (const auto& shstub : ctx.layout.shstubs())     patch_entry(shstub);
    patch_entry(handler_bin);

    session.save(out_path);
    std::cout << Color::GREEN << "written to " << out_path.string() << Color::RST << '\n';
}

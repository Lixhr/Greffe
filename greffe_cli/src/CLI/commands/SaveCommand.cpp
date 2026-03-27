#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "colors.hpp"
#include <iostream>

std::string_view SaveCommand::name()        const { return "save"; }
std::string_view SaveCommand::description() const { return "Save greffes"; }

void SaveCommand::execute(CLIContext& ctx, const Args&) {
    auto path = ctx.pinfo.getProjectDir() / ".greffe";
    ctx.targets.save(path, ctx.bin_base, ctx.patch_base);
    std::cout << Color::GREY << "saved to " << path.string() << Color::RST << '\n';
}

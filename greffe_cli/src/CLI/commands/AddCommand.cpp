#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "Target.hpp"
#include "patch/TrampolineBuilder.hpp"
#include "patch/arch/StubsFactory.hpp"
#include "patch/arch/RelocatorFactory.hpp"
#include "patch_utils.hpp"
#include "cli_fmt.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

void create_handler_stub(const Target& t, const ProjectInfo& pinfo) {
    namespace fs = std::filesystem;

    auto dir = pinfo.getProjectDir() / "handlers";
    fs::create_directories(dir);

    auto path = dir / (sanitize(t.name()) + ".c");
    if (fs::exists(path))
        return;

    static const std::pair<std::string_view, std::string_view> attr_table[] = {
        { "thumb", "__attribute__((target(\"thumb\")))" },
    };

    std::string_view attr;
    for (const auto& c : t.context()) {
        if (c.ea != t.ea()) continue;
        for (const auto& [mode, a] : attr_table)
            if (c.mode == mode) { attr = a; break; }
        break;
    }

    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("cannot create " + path.string());

    if (!attr.empty())
        f << attr << '\n';
    f << "void handler_" << sanitize(t.name()) << "(void)\n{\n}\n";
}

std::string_view AddCommand::name()        const { return "add"; }
std::string_view AddCommand::description() const { return "Register a greffe"; }

void AddCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.empty())
        throw std::runtime_error("usage: add <target> [target ...]");

    for (const auto& arg : args) {
        const Target& t = ctx.targets.add(arg);
        auto stubs     = StubsFactory::create(t, ctx.pinfo);
        auto relocator = RelocatorFactory::create(t, ctx.pinfo);
        try {
            TrampolineBuilder::validate(t, *stubs, *relocator);
        } catch (...) {
            ctx.targets.remove(t.name());
            throw;
        }
        std::cout << TargetView{ t, ctx.pinfo.getBits() };
        create_handler_stub(t, ctx.pinfo);
    }
}

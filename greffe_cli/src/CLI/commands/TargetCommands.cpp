#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "Target.hpp"
#include "HandlerCompiler.hpp"
#include "PatchSession.hpp"
#include "patch/TrampolineBuilder.hpp"
#include "patch/arch/StubsFactory.hpp"
#include "patch/arch/RelocatorFactory.hpp"
#include "patch_utils.hpp"
#include "colors.hpp"
#include <cinttypes>
#include <filesystem>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <readline/readline.h>
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
    const auto& targets = ctx->targets.targets();
    const std::string prefix = args.empty() ? "" : args.back();

    for (const auto& t : targets) {
        const std::string& name = t.name();

        if (name.rfind(prefix, 0) == 0) {
            result.push_back(name);
        }
    }

    return result;
}

std::string_view SaveCommand::name()        const { return "save"; }
std::string_view SaveCommand::description() const { return "Save greffes"; }

void SaveCommand::execute(CLIContext& ctx, const Args&) {
    auto path = ctx.pinfo.getProjectDir() / ".greffe";
    ctx.targets.save(path, ctx.bin_base, ctx.patch_base);
    std::cout << Color::GREY << "saved to " << path.string() << Color::RST << '\n';
}

std::string_view ListCommand::name()        const { return "list"; }
std::string_view ListCommand::description() const { return "List registered greffes"; }

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
std::string_view PatchCommand::description() const { return "Compile handlers and apply all greffes"; }

void PatchCommand::execute(CLIContext& ctx, const Args&) {
    if (ctx.targets.targets().empty())
        throw std::runtime_error("nothing to patch");

    if (!ctx.patch_base)
        throw std::runtime_error("patch_base is not set (use: set patch_base <addr>)");

    int      bits = ctx.pinfo.getBits();
    unsigned w    = static_cast<unsigned>(bits / 4);

    std::cout << "  bin_base   : 0x" << std::hex << std::setfill('0')
              << std::setw(w) << ctx.bin_base   << '\n'
              << "  patch_base : 0x" << std::setw(w) << *ctx.patch_base
              << std::dec << '\n';

    char* raw = readline("Proceed? [y/N] ");
    std::string answer;
    if (raw) { answer = raw; free(raw); }

    if (answer != "y" && answer != "Y") {
        std::cout << Color::GREY << "aborted" << Color::RST << '\n';
        return;
    }

    auto outfile = std::filesystem::path(ctx.pinfo.getBinPath().string() + ".greffé");

    HandlerBin handler_bin = HandlerCompiler::build(ctx.targets.targets(), ctx.pinfo);
    PatchSession::run(ctx.targets.targets(), handler_bin,
                      *ctx.patch_base, ctx.bin_base,
                      ctx.pinfo, outfile);
}

std::string_view SetCommand::name()        const { return "set"; }
std::string_view SetCommand::description() const { return "Set bin_base or patch_base"; }

void SetCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.size() != 2)
        throw std::runtime_error("usage: set [bin_base|patch_base] <addr>");

    const std::string& field = args[0];
    uint64_t value;
    try {
        std::size_t pos = 0;
        value = std::stoull(args[1], &pos, 0);
        if (pos != args[1].size())
            throw std::invalid_argument("");
    } catch (const std::exception&) {
        throw std::runtime_error("invalid address: " + args[1]);
    }

    if (field == "bin_base") {
        ctx.bin_base = value;
        std::cout << Color::GREY << "bin_base = 0x" << std::hex << value
                  << std::dec << Color::RST << '\n';
    } else if (field == "patch_base") {
        ctx.patch_base = value;
        std::cout << Color::GREY << "patch_base = 0x" << std::hex << value
                  << std::dec << Color::RST << '\n';
    } else {
        throw std::runtime_error("unknown field '" + field + "'");
    }
}

std::vector<std::string> SetCommand::complete(const CLIContext* /*ctx*/, const Args& args) const {
    static const std::vector<std::string> fields = {"bin_base", "patch_base"};
    if (args.empty())
        return fields;
    if (args.size() == 1) {
        std::vector<std::string> result;
        for (const auto& f : fields)
            if (f.rfind(args[0], 0) == 0)
                result.push_back(f);
        return result;
    }
    return {};
}

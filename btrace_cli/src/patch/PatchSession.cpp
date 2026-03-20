#include "PatchSession.hpp"
#include "TrampolineBuilder.hpp"
#include "BinaryPatcher.hpp"
#include "patch_utils.hpp"
#include "arch/StubsFactory.hpp"
#include "arch/RelocatorFactory.hpp"
#include "colors.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

static std::string hex(uint64_t v) {
    char buf[19];
    snprintf(buf, sizeof(buf), "0x%lx", v);
    return buf;
}

void PatchSession::run(const std::vector<Target>&   targets,
                       const HandlerBin&             handler_bin,
                       uint64_t                      patch_base,
                       uint64_t                      bin_base,
                       const ProjectInfo&            pinfo,
                       const std::filesystem::path&  outfile)
{
    if (targets.empty())
        throw std::runtime_error("PatchSession: no targets");

    if (patch_base < bin_base)
        throw std::runtime_error("PatchSession: patch_base (" + hex(patch_base)
                                 + ") < bin_base (" + hex(bin_base) + ")");

    std::filesystem::copy_file(pinfo.getBinPath(), outfile,
                               std::filesystem::copy_options::overwrite_existing);

    if (!handler_bin.bytes().empty())
        BinaryPatcher::patch(outfile, patch_base - bin_base, handler_bin.bytes());

    uint64_t current_addr = patch_base + static_cast<uint64_t>(handler_bin.size());

    for (size_t i = 0; i < targets.size(); ++i) {
        const Target& t = targets[i];

        auto stubs     = StubsFactory::create(t, pinfo);
        auto relocator = RelocatorFactory::create(t, pinfo);

        auto trampoline = TrampolineBuilder::build(
            t,
            handler_bin.handler_addr("handler_" + sanitize(t.name()), patch_base),
            current_addr,
            *stubs,
            *relocator);

        uint64_t trampoline_offset = current_addr - bin_base;
        BinaryPatcher::patch(outfile, trampoline_offset, trampoline);

        auto hook = stubs->branch(t.ea(), current_addr);
        uint64_t hook_offset = t.ea() - bin_base;
        BinaryPatcher::patch(outfile, hook_offset, hook);

        std::cout << Color::CYAN << t.name() << Color::RST
                  << "  trampoline @ " << hex(current_addr)
                  << "  hook @ "       << hex(t.ea())
                  << "  (" << trampoline.size() << " B)\n";

        current_addr += trampoline.size();
    }

    std::cout << Color::GREEN << "written to " << outfile.string()
              << Color::RST << '\n';
}

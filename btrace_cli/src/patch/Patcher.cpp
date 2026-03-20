#include "Patcher.hpp"
#include "BinaryPatcher.hpp"
#include "arch/RelocatorFactory.hpp"
#include <stdexcept>

static std::vector<uint8_t> hex_decode(const std::string& hex) {
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
        out.push_back(static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    return out;
}

std::vector<uint8_t> Patcher::collect_input(const Target& t) {
    std::vector<uint8_t> out;
    for (const auto& c : t.context()) {
        if (c.ea < t.ea()) continue;
        if (c.raw.size() % 2 != 0)
            throw std::runtime_error("malformed raw bytes for " + t.name());
        auto b = hex_decode(c.raw);
        out.insert(out.end(), b.begin(), b.end());
    }
    return out;
}

PatchResult Patcher::apply(const Target&                t,
                            uint64_t                     trampoline_addr,
                            uint64_t                     file_offset,
                            const ProjectInfo&           pinfo,
                            const std::filesystem::path& outfile) {
    const ContextEntry* entry = nullptr;
    for (const auto& c : t.context())
        if (c.ea == t.ea()) { entry = &c; break; }

    if (!entry)
        throw std::runtime_error("no instruction data for " + t.name());

    auto input_bytes = collect_input(t);
    size_t target_size = entry->raw.size() / 2;

    auto relocator = RelocatorFactory::create(t, pinfo);
    auto reloc     = relocator->relocate(input_bytes, target_size, t.ea(), trampoline_addr);

    BinaryPatcher::patch(outfile, file_offset, reloc.bytes);

    return PatchResult{ std::move(reloc), trampoline_addr, file_offset };
}

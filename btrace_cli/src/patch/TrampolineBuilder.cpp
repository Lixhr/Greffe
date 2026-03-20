#include "TrampolineBuilder.hpp"
#include "Patcher.hpp"

#include <stdexcept>

std::vector<uint8_t> TrampolineBuilder::build(const Target& t,
                                               uint64_t      handler_addr,
                                               uint64_t      trampoline_addr,
                                               IArchStubs&   stubs,
                                               IRelocator&   relocator)
{
    size_t hook_size = stubs.branch(t.ea(), trampoline_addr).size();

    size_t   n_bytes   = 0;
    uint64_t return_ea = t.ea();
    for (const auto& c : t.context()) {
        if (c.ea < t.ea()) continue;
        if (c.raw.size() % 2 != 0)
            throw std::runtime_error("TrampolineBuilder: malformed raw bytes for " + t.name());
        n_bytes   += c.raw.size() / 2;
        return_ea  = c.ea + c.raw.size() / 2;
        if (n_bytes >= hook_size) break;
    }
    if (n_bytes < hook_size)
        throw std::runtime_error("TrampolineBuilder: not enough context to cover hook for " + t.name());

    std::vector<uint8_t> result;
    uint64_t cur = trampoline_addr;

    auto append = [&](std::vector<uint8_t> part, const char* label) {
        if (part.empty())
            throw std::runtime_error(std::string("TrampolineBuilder: ")
                                     + label + " produced empty bytes for " + t.name());
        cur += part.size();
        result.insert(result.end(),
                      std::make_move_iterator(part.begin()),
                      std::make_move_iterator(part.end()));
    };

    append(stubs.save_ctx(cur),           "save_ctx");
    append(stubs.call(cur, handler_addr), "call");
    append(stubs.restore_ctx(cur),        "restore_ctx");

    auto input = Patcher::collect_input(t);
    auto reloc  = relocator.relocate(input, n_bytes, t.ea(), cur);
    if (reloc.bytes.empty())
        throw std::runtime_error("TrampolineBuilder: relocation produced no bytes for " + t.name());

    cur += reloc.bytes.size();
    result.insert(result.end(),
                  std::make_move_iterator(reloc.bytes.begin()),
                  std::make_move_iterator(reloc.bytes.end()));

    append(stubs.branch(cur, return_ea), "return branch");

    return result;
}

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

    size_t branch_size = stubs.branch(cur, cur).size();
    std::vector<uint8_t> placeholder(branch_size, 0);

    auto input = Patcher::collect_input(t);
    auto reloc  = relocator.relocate(input, n_bytes, t.ea(), cur, placeholder);
    if (reloc.bytes.empty())
        throw std::runtime_error("TrampolineBuilder: relocation produced no bytes for " + t.name());
    if (reloc.insns_size + branch_size > reloc.bytes.size())
        throw std::runtime_error("TrampolineBuilder: relocation layout inconsistency for " + t.name());

    uint64_t branch_addr = cur + static_cast<uint64_t>(reloc.insns_size);
    auto branch_bytes = stubs.branch(branch_addr, return_ea);
    if (branch_bytes.size() != branch_size)
        throw std::runtime_error("TrampolineBuilder: branch size mismatch for " + t.name());

    std::copy(branch_bytes.begin(), branch_bytes.end(),
              reloc.bytes.begin() + static_cast<ptrdiff_t>(reloc.insns_size));

    cur += reloc.bytes.size();
    result.insert(result.end(),
                  std::make_move_iterator(reloc.bytes.begin()),
                  std::make_move_iterator(reloc.bytes.end()));

    return result;
}

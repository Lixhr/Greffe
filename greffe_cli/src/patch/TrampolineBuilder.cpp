#include "TrampolineBuilder.hpp"
#include "Patcher.hpp"

#include <stdexcept>


struct Coverage {
    size_t   n_bytes;
    uint64_t return_ea;
};

static Coverage compute_coverage(const Target&              t,
                                 size_t                     hook_size,
                                 IRelocator&                relocator,
                                 const std::vector<uint8_t>& input) {
    size_t target_insn_size = 0;
    for (const auto& c : t.context()) {
        if (c.ea == t.ea()) { target_insn_size = c.raw.size() / 2; break; }
    }
    if (target_insn_size == 0)
        throw std::runtime_error("TrampolineBuilder: target instruction not found for " + t.name());

    if (hook_size > target_insn_size) {
        std::vector<uint8_t> first_insn(input.begin(),
                                        input.begin() + static_cast<ptrdiff_t>(target_insn_size));
        if (relocator.is_branch(first_insn, t.ea()))
            throw std::runtime_error(
                "TrampolineBuilder: hook (" + std::to_string(hook_size) + " bytes) "
                "exceeds branch instruction (" + std::to_string(target_insn_size) + " bytes) "
                "at " + t.name());
    }

    size_t   n_bytes   = 0;
    uint64_t return_ea = t.ea();
    for (const auto& c : t.context()) {
        if (c.ea < t.ea()) continue;
        if (c.raw.size() % 2 != 0)
            throw std::runtime_error("TrampolineBuilder: malformed raw bytes for " + t.name());

        if (c.ea > t.ea() && n_bytes < hook_size && c.is_xref_target) {
            char buf[19];
            snprintf(buf, sizeof(buf), "0x%lx", c.ea);
            throw std::runtime_error(
                "TrampolineBuilder: hook for " + t.name() + " would overwrite "
                + buf + " which is a code xref");

        }
        n_bytes   += c.raw.size() / 2;
        return_ea  = c.ea + c.raw.size() / 2;
        if (n_bytes >= hook_size) break;
    }
    if (n_bytes < hook_size)
        throw std::runtime_error("TrampolineBuilder: not enough context to cover hook for " + t.name());

    return { n_bytes, return_ea };
}


void TrampolineBuilder::validate(const Target& t, IArchStubs& stubs, IRelocator& relocator) {
    size_t target_insn_size = 0;
    for (const auto& c : t.context()) {
        if (c.ea == t.ea()) { target_insn_size = c.raw.size() / 2; break; }
    }
    if (target_insn_size == 0)
        throw std::runtime_error("TrampolineBuilder: target instruction not found for " + t.name());

    size_t hook_size = stubs.branch_size_max();
    if (hook_size > target_insn_size) {
        auto input = Patcher::collect_input(t);
        std::vector<uint8_t> first_insn(input.begin(),
                                        input.begin() + static_cast<ptrdiff_t>(target_insn_size));
        if (relocator.is_branch(first_insn, t.ea()))
            throw std::runtime_error(
                "TrampolineBuilder: hook (" + std::to_string(hook_size) + " bytes) "
                "exceeds branch instruction (" + std::to_string(target_insn_size) + " bytes) "
                "at " + t.name());

        size_t n_bytes = 0;
        for (const auto& c : t.context()) {
            if (c.ea < t.ea()) continue;
            if (c.ea > t.ea() && n_bytes < hook_size && c.is_xref_target) {
                char buf[19];
                snprintf(buf, sizeof(buf), "0x%lx", c.ea);
                throw std::runtime_error(
                    "TrampolineBuilder: hook for " + t.name() + " would overwrite "
                    + buf + " which is a code xref target");
            }

            n_bytes += c.raw.size() / 2;
            if (n_bytes >= hook_size) break;
        }
    }
}

std::vector<uint8_t> TrampolineBuilder::build(const Target& t,
                                               uint64_t      handler_addr,
                                               uint64_t      trampoline_addr,
                                               IArchStubs&   stubs,
                                               IRelocator&   relocator) {
    auto   input     = Patcher::collect_input(t);
    size_t hook_size = stubs.branch(t.ea(), trampoline_addr).size();

    auto [n_bytes, return_ea] = compute_coverage(t, hook_size, relocator, input);

    std::vector<uint8_t> result;
    uint64_t             cur = trampoline_addr;

    auto emit = [&](std::vector<uint8_t> part, const char* label) {
        if (part.empty())
            throw std::runtime_error(std::string("TrampolineBuilder: ")
                                     + label + " produced empty bytes for " + t.name());
        cur += part.size();
        result.insert(result.end(),
                      std::make_move_iterator(part.begin()),
                      std::make_move_iterator(part.end()));
    };

    emit(stubs.save_ctx(cur),           "save_ctx");
    emit(stubs.call(cur, handler_addr), "call");
    emit(stubs.restore_ctx(cur),        "restore_ctx");

    size_t branch_size = stubs.branch_size_max();

    auto reloc = relocator.relocate(input, n_bytes, t.ea(), cur,
                                    std::vector<uint8_t>(branch_size, 0));
    if (reloc.bytes.empty())
        throw std::runtime_error("TrampolineBuilder: relocation produced no bytes for " + t.name());

    if (!reloc.ends_with_branch) {
        if (reloc.insns_size + branch_size > reloc.bytes.size())
            throw std::runtime_error("TrampolineBuilder: relocation layout inconsistency for " + t.name());

        uint64_t branch_addr  = cur + static_cast<uint64_t>(reloc.insns_size);
        auto     branch_bytes = stubs.branch(branch_addr, return_ea);
        if (branch_bytes.size() > branch_size)
            throw std::runtime_error("TrampolineBuilder: branch size mismatch for " + t.name());

        std::copy(branch_bytes.begin(), branch_bytes.end(),
                  reloc.bytes.begin() + static_cast<ptrdiff_t>(reloc.insns_size));
    }

    cur += reloc.bytes.size();
    result.insert(result.end(),
                  std::make_move_iterator(reloc.bytes.begin()),
                  std::make_move_iterator(reloc.bytes.end()));

    return result;
}

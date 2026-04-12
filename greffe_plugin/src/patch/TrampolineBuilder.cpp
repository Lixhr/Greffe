#include "TrampolineBuilder.hpp"
#include "PatchSession.hpp"
#include <stdexcept>
#include <sstream>

#include <bytes.hpp>
#include <funcs.hpp>
#include <xref.hpp>
#include "utils.hpp"

PatchBranch TrampolineBuilder::branch_to_trampoline(PatchPlan& plan) {
    const Target& t     = plan.target;
    IArchStubs&   stubs = *plan.stubs;

    std::vector<uint8_t> branch = stubs.branch(t.ea(), plan.addr());

    func_t *func = get_func(static_cast<ea_t>(t.ea()));
    if (!func) {
        std::ostringstream ss;
        ss << "no function at 0x" << std::hex << t.ea();
        throw std::runtime_error(ss.str());
    }

    size_t len = 0;
    std::vector<ContextEntry> relocd;
    ea_t cur = static_cast<ea_t>(t.ea());

    while (true) {
        if (!func_contains(func, cur))
            throw std::runtime_error("Patched branch overlaps end of function");

        asize_t size = get_item_size(cur);
        if (size == 0) {
            std::ostringstream ss;
            ss << "failed to get item size at 0x" << std::hex << cur;
            throw std::runtime_error(ss.str());
        }

        ContextEntry entry;
        entry.ea  = static_cast<uint64_t>(cur);
        entry.raw.resize(size);
        if (get_bytes(entry.raw.data(), size, cur) == -1) {
            std::ostringstream ss;
            ss << "failed to read bytes at 0x" << std::hex << cur;
            throw std::runtime_error(ss.str());
        }
        entry.is_xref_target = get_first_fcref_to(cur) != BADADDR;

        if (cur > static_cast<ea_t>(t.ea()) && entry.is_xref_target)
            throw std::runtime_error("Patched branch overlaps a CODE XREF");

        len += size;
        relocd.push_back(std::move(entry));
        cur += size;

        if (len >= branch.size()) {
            plan.trampoline_ret_addr = static_cast<uint64_t>(cur);
            plan.relocd_instr        = std::move(relocd);
            return PatchBranch(t.ea(), std::move(branch));
        }
    }
}

size_t  TrampolineBuilder::init_trampoline(PatchPlan &plan,
                                           const SharedStub &shstub) {
    uint8_t *ptr = nullptr;
    plan.bytes() = plan.stubs->trampoline_init(plan.addr(),
                                               shstub.addr(),
                                               &ptr);
    plan.handler_offset = static_cast<size_t>(ptr - plan.bytes().data());
    return plan.bytes().size();
}

size_t TrampolineBuilder::relocate_and_branch_back(PatchPlan& plan) {
    uint64_t dest_addr = plan.addr() + plan.bytes().size();

    auto tail = plan.stubs->relocate_and_branch_back(plan.relocd_instr,
                                                     dest_addr,
                                                     plan.trampoline_ret_addr);
    plan.bytes().insert(plan.bytes().end(), tail.begin(), tail.end());
    return tail.size();
}

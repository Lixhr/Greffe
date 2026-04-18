#include "TrampolineBuilder.hpp"
#include <stdexcept>
#include <sstream>
#include <bytes.hpp>
#include <xref.hpp>
#include "utils.hpp"
#include "offset.hpp"

PatchBranch TrampolineBuilder::branch_to_trampoline(PatchPlan& plan) {
    IArchStubs& stubs = *plan.stubs;

    std::vector<uint8_t> branch = stubs.branch(plan.target_ea, plan.ea());

    size_t len = 0;
    std::vector<ContextEntry> relocd;
    ea_t cur = plan.target_ea;

    while (true) {
        if (!is_code(get_flags(cur)))
            throw std::runtime_error("Patched branch overlaps non-instruction");

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

        if (cur > plan.target_ea && entry.is_xref_target)
            throw std::runtime_error("Patched branch overlaps a CODE XREF");

        len += size;
        relocd.push_back(std::move(entry));
        cur += size;

        if (len >= branch.size()) {
            plan.trampoline_ret_addr = static_cast<ea_t>(cur);
            plan.relocd_instr        = std::move(relocd);
            return PatchBranch(plan.target_ea, std::move(branch), plan.trampoline_ret_addr);
        }
    }
}

size_t  TrampolineBuilder::init_trampoline(PatchPlan &plan,
                                           const SharedStub &shstub) {
    uint8_t *ptr = nullptr;
    plan.bytes() = plan.stubs->trampoline_init(plan.ea(),
                                               shstub.ea(),
                                               &ptr);
    plan.handler_ptr_addr = plan.ea() + static_cast<size_t>(ptr - plan.bytes().data());
    return plan.bytes().size();
}

size_t TrampolineBuilder::relocate_and_branch_back(PatchPlan& plan) {
    uint64_t dest_addr = plan.ea() + plan.bytes().size();

    auto tail = plan.stubs->relocate_and_branch_back(plan.relocd_instr,
                                                     dest_addr,
                                                     plan.trampoline_ret_addr);
    plan.bytes().insert(plan.bytes().end(), tail.begin(), tail.end());
    return tail.size();
}

#include "TrampolineBuilder.hpp"
#include "PatchSession.hpp"
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <iostream>

void TrampolineBuilder::patch_branches(PatchSession& session,
                                            const std::vector<PatchPlan>& plans) {
    const PatchPlan* last = nullptr;

    for (const auto& plan : plans) {
        if (!last || plan.target.ea() > last->target.ea())
            last = &plan;

        session.patch(plan.target.ea(), plan.branch_instr);
    }
}

void TrampolineBuilder::branch_to_trampoline(PatchPlan& plan) {
    const Target& t     = plan.target;
    IArchStubs&   stubs = *plan.stubs;

    std::vector<uint8_t> branch = stubs.branch(t.ea(), plan.trampoline_addr);

    const auto& ctx = t.context();
    auto it = std::find_if(ctx.begin(), ctx.end(),
        [&t](const ContextEntry& c) { return c.ea == t.ea(); });

    if (it == ctx.end())
        throw std::runtime_error("Target instruction not found on context ??");

    size_t len = 0;
    std::vector<const ContextEntry*> relocd_indices;

    while (it != ctx.end()) {
        if (it->ea > t.ea() && it->is_xref_target)
            throw std::runtime_error("Patched branch overlaps a CODE XREF");

        len += it->raw.size();
        relocd_indices.push_back(&*it);
        ++it;

        if (len >= branch.size()) {
            plan.trampoline_ret_addr   = it->ea;
            plan.relocd_instr  = std::move(relocd_indices);
            plan.branch_instr          = std::move(branch);
            return;
        }
    }

    throw std::runtime_error("Patched branch overlaps end of function");
}

size_t TrampolineBuilder::relocate_instructions(PatchPlan& plan) {
    std::shared_ptr<IArchStubs> &stubs = plan.stubs;
    uint64_t    dest_addr              = plan.trampoline_addr + plan.trampoline.size();
    size_t      total                  = 0;

    for (const ContextEntry* e : plan.relocd_instr) {
        auto relocated = stubs->relocate(*e, dest_addr + total);
        plan.trampoline.insert(plan.trampoline.end(), relocated.begin(), relocated.end());
        total += relocated.size();
    }
    return total;
}

size_t  TrampolineBuilder::init_trampoline(PatchPlan &plan,
                                           const SharedStub &shstub) {

    std::shared_ptr<IArchStubs> &stubs = plan.stubs;

    plan.trampoline = stubs->trampoline_init(plan.trampoline_addr, 
                                            shstub.addr(), 
                                            &plan.trampoline_ret);
    return plan.trampoline.size();
}

size_t  TrampolineBuilder::branch_back(PatchPlan& plan) {
    std::shared_ptr<IArchStubs> &stubs = plan.stubs;
    uint64_t br_addr = plan.trampoline_addr + plan.trampoline.size();

    std::vector<uint8_t> branch = stubs->branch(br_addr, plan.trampoline_ret_addr);
    plan.trampoline.insert(plan.trampoline.end(), branch.begin(), branch.end());

    return branch.size();
}

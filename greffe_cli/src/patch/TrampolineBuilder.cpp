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

    std::vector<uint8_t> branch = stubs.branch(t.ea(), plan.addr());

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
            plan.trampoline_ret_addr  = it->ea;
            plan.relocd_instr         = std::move(relocd_indices);
            plan.branch_instr         = std::move(branch);
            return;
        }
    }

    throw std::runtime_error("Patched branch overlaps end of function");
}

size_t  TrampolineBuilder::init_trampoline(PatchPlan &plan,
                                           const SharedStub &shstub) {
    plan.bytes() = plan.stubs->trampoline_init(plan.addr(),
                                               shstub.addr(),
                                               &plan.trampoline_ret);
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

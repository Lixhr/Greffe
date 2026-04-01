#include "TrampolineBuilder.hpp"
#include "PatchSession.hpp"
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <iostream>

uint64_t TrampolineBuilder::patch_branches(PatchSession& session,
                                            const std::vector<PatchPlan>& plans) {
    const PatchPlan* last = nullptr;

    for (const auto& plan : plans) {
        if (!last || plan.target.ea() > last->target.ea())
            last = &plan;

        session.patch(plan.target.ea(), plan.branch_instr);
    }

    return last->trampoline_addr + last->stubs->branch_placeholder_size();
}

void TrampolineBuilder::branch_init(PatchPlan& plan) {
    const Target& t     = plan.target;
    IArchStubs&   stubs = *plan.stubs;

    std::vector<uint8_t> branch = stubs.branch(t.ea(), plan.trampoline_addr);

    const auto& ctx = t.context();
    auto it = std::find_if(ctx.begin(), ctx.end(),
        [&t](const ContextEntry& c) { return c.ea == t.ea(); });

    if (it == ctx.end())
        throw std::runtime_error("Target instruction not found on context ??");

    size_t len = 0;
    std::vector<size_t> relocd_indices;

    while (it != ctx.end()) {
        if (it->ea > t.ea() && it->is_xref_target)
            throw std::runtime_error("Patched branch overlaps a CODE XREF");

        len += it->raw.size();
        relocd_indices.push_back(static_cast<size_t>(std::distance(ctx.begin(), it)));
        ++it;

        if (len >= branch.size()) {
            plan.trampoline_ret_addr   = it->ea;
            plan.relocd_instr_indices  = std::move(relocd_indices);
            plan.branch_instr          = std::move(branch);
            return;
        }
    }

    throw std::runtime_error("Patched branch overlaps end of function");
}

std::vector<uint8_t> TrampolineBuilder::init_trampoline(const PatchPlan&  plan,
                                               uint64_t         /*handler_addr*/,
                                               IRelocator&      /*relocator*/) {
    plan.stubs->save_ctx(plan.trampoline_addr);

    throw std::runtime_error("TrampolineBuilder::build: not implemented");
}

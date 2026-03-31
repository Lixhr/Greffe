#include "TrampolineBuilder.hpp"
#include "PatchSession.hpp"
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <iostream>

uint64_t TrampolineBuilder::patch_branches(PatchSession &session, 
                                       const std::vector<Target> &targets) {
    const Target *last_target = NULL;

    for (const auto &target : targets) {
        // keep track of the last trampoline
        if (!last_target || target.ea() > last_target->ea())
            last_target = &target;

        session.patch(target.ea(), target.branch_instr());
    }

    return (last_target->trampoline_addr() 
    + last_target->stubs().branch_placeholder_size()); // current writing pointer
}

void TrampolineBuilder::branch_init(Target& t) {
    // generate a branch from the target to the trampoline
    IArchStubs &stubs = t.stubs();
    std::vector<uint8_t> branch = stubs.branch(t.ea(), t.trampoline_addr());


    // get our target
    auto it = std::find_if(t.context().begin(), t.context().end(),                                                                                                                                
    [&t](const ContextEntry& c) { return c.ea == t.ea() ; }); 

    if (it == t.context().end())
        throw std::runtime_error("Target instruction not found on context ??");


    size_t len = 0;
    std::vector<const ContextEntry *> relocd_instrs;

    while (it != t.context().end()) {
        // avoids overwriting instructions which is CODE XREFed
        // the first instr is ignored (func start)
        if (it->ea > t.ea() && it->is_xref_target)
            throw std::runtime_error("Patched branch overlaps a CODE XREF");

        len += it->raw.size();
        relocd_instrs.push_back(&(*it));
        it ++;
        
        if (len >= branch.size()) {
            t.setTrampolineRetAddr(it->ea);
            t.setRelocdInstrs(std::move(relocd_instrs));
            t.setBranchInstr(std::move(branch));
            return ;
        }
    }

    throw std::runtime_error("Patched branch overlaps end of function");
}

std::vector<uint8_t> TrampolineBuilder::build(const Target&  /*t*/,
                                               uint64_t      /*handler_addr*/,
                                               uint64_t      /*trampoline_addr*/,
                                               IArchStubs&   /*stubs*/,
                                               IRelocator&   /*relocator*/) {
    throw std::runtime_error("TrampolineBuilder::build: not implemented");
}
